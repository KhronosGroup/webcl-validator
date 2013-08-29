#include "WebCLHelper.hpp"

#include "clang/AST/Expr.h"

#include "clang/AST/ASTContext.h"
#include "clang/AST/Expr.h"
#include "clang/Frontend/CompilerInstance.h"
#include "clang/Rewrite/Core/Rewriter.h"
#include "clang/Lex/Lexer.h"
#include "clang/Tooling/Refactoring.h"

#include <sstream>

WebCLRewriter::WebCLRewriter(clang::CompilerInstance &instance,
                             clang::Rewriter &rewriter)
    : instance_(instance)
    , rewriter_(rewriter)
    , isFilteredRangesDirty_(false)
{
  
}


/// \brief Finds source location of next token which starts with given char
clang::SourceLocation WebCLRewriter::findLocForNext(clang::SourceLocation startLoc, char charToFind) {
  
  // if we could use matchers to find these locations would be better
  const clang::SourceManager &SM = instance_.getSourceManager();
  std::pair<clang::FileID, unsigned> locInfo = SM.getDecomposedLoc(startLoc);
  bool invalidTemp = false;
  clang::StringRef file = SM.getBufferData(locInfo.first, &invalidTemp);
  assert(invalidTemp == false);
  const char *tokenBegin = file.data() + locInfo.second;
  
  clang::Lexer lexer(SM.getLocForStartOfFile(locInfo.first), instance_.getLangOpts(),
                     file.begin(), tokenBegin, file.end());
  
  // just raw find for next semicolon and get SourceLocation
  const char* endOfFile = SM.getCharacterData(SM.getLocForEndOfFile(locInfo.first));
  const char* charPos = SM.getCharacterData(startLoc);
  
  while (*charPos != charToFind && charPos < endOfFile) {
    clang::SourceLocation currLoc = lexer.getSourceLocation(charPos);
    int currLength = clang::Lexer::MeasureTokenLength(currLoc, SM, instance_.getLangOpts());
    // at least advance one char
    if (currLength == 0) {
      currLength = 1;
    }
    charPos += currLength;
    // std::cerr << "Curr token:" << rewriter_.getRewrittenText(clang::SourceRange(currLoc, currLoc)) << "\n";
  };
  
  return lexer.getSourceLocation(charPos);
}

void WebCLRewriter::removeText(clang::SourceRange range) {
  isFilteredRangesDirty_ = true;
  DEBUG( std::cerr << "Remove SourceLoc " << range.getBegin().getRawEncoding() << ":" << range.getEnd().getRawEncoding() << " " << getOriginalText(range) << "\n"; );
  replaceText(range, "");
}

void WebCLRewriter::insertText(clang::SourceLocation location, const std::string &text)
{
    replaceText(clang::SourceRange(location, location), text);
}

void WebCLRewriter::replaceText(clang::SourceRange range, const std::string &text)
{
  isFilteredRangesDirty_ = true;
  int rawStart = range.getBegin().getRawEncoding();
  int rawEnd = range.getEnd().getRawEncoding();
  modifiedRanges_[ModifiedRange(rawStart, rawEnd)] = text;
  DEBUG( std::cerr << "Replace SourceLoc " << rawStart << ":" << rawEnd << " " << getOriginalText(range) << " with: " << text << "\n"; );
}

std::string WebCLRewriter::getOriginalText(clang::SourceRange range)
{
    return rewriter_.getRewrittenText(range);
}

// NOTE: Current implementation is really limited. Better could be to have stack of
//       changes all the time in the system and apply them less intelligent way,
//       in that case ranges could not be used in internal representation, but offset + length
//       instead.
std::string WebCLRewriter::getTransformedText(clang::SourceRange range) {

  int rawStart = range.getBegin().getRawEncoding();
  int rawEnd = range.getEnd().getRawEncoding();
  
  RangeModifications::iterator start =
  modifiedRanges_.lower_bound(ModifiedRange(rawStart, rawStart));
  
  RangeModifications::iterator end =
  modifiedRanges_.upper_bound(ModifiedRange(rawEnd, rawEnd));
  
  std::string retVal;
  
  // if there is exact match, return it
  if (modifiedRanges_.count(ModifiedRange(rawStart, rawEnd))) {
    retVal = modifiedRanges_[ModifiedRange(rawStart, rawEnd)];
  } else if (start == end) {
    // if no matches and start == end get from rewriter
    retVal = getOriginalText(range);
  } else {
    
    // splits source rawStart and rawEnd to pieces which are read from either modified areas table or from original source
    
    // TODO: refactor to use filteredModifiedRanges() like flushing does
    // ===== start of filtering nested modified ranges =====
    int currentStart = -1;
    int biggestEnd = -1;
    std::vector<ModifiedRange> filteredRanges;
    
    // find out only toplevel ranges without nested areas
    for (RangeModifications::iterator i = start; i != end; i++) {
      
      DEBUG( std::cerr << "Range " << i->first.first << ":" << i->first.second << " = " << i->second << "\n"; );
      
      if (i->first.first < biggestEnd) {
        DEBUG( std::cerr << "Skipping: " << "\n"; );
        continue;
      }
      
      if (currentStart != i->first.first) {
        if (currentStart != -1) {
          // This is a big vague because
          // area size might be zero in location ids
          // if current area cannot overlap earlier or if no earlier or it does not overlap => add area
          if (currentStart != biggestEnd ||
              filteredRanges.empty() ||
              currentStart != filteredRanges.back().second) {
            DEBUG( std::cerr << "Added range: " << currentStart << ":" << biggestEnd << "\n"; );
            filteredRanges.push_back(ModifiedRange(currentStart, biggestEnd));
          }
        }
        currentStart = i->first.first;
      }
      
      biggestEnd = i->first.second;
    }
    
    // if last currentStart, biggestEnd is not nested... this is a bit nasty because source location start and end might be
    // the same which is a big ambigious...
    if (currentStart != biggestEnd ||
        filteredRanges.empty() ||
        currentStart != filteredRanges.back().second) {
      DEBUG( std::cerr << "Added range: " << currentStart << ":" << biggestEnd << "\n"; );
      filteredRanges.push_back(ModifiedRange(currentStart, biggestEnd));
    }
    
    // ===== end of filtering =====
    
    // NOTE: easier way to do following could be to get first the whole range as string and then just replace
    // filtered modified ranges, whose original length and start index is easy to find out
    
    std::stringstream result;
    // concat pieces from original sources and replacements from rawStart to rawEnd
    int current = rawStart;
    bool offsetStartLoc = false;
    
    for (unsigned i = 0; i < filteredRanges.size(); i++) {
      ModifiedRange range = filteredRanges[i];
      if (range.first != current) {
        // get range from rewriter current, range.first
        clang::SourceLocation startLoc = clang::SourceLocation::getFromRawEncoding(current);
        clang::SourceLocation endLoc = clang::SourceLocation::getFromRawEncoding(range.first);
        int endLocSize = rewriter_.getRangeSize(clang::SourceRange(endLoc, endLoc));
        int startLocSize = 0;
        if (offsetStartLoc) {
          startLocSize = rewriter_.getRangeSize(clang::SourceRange(startLoc, startLoc));
        }
        // get source and exclude start and end tokens in case if they were already included
        // in modified range.
        std::string source = getOriginalText(clang::SourceRange(startLoc, endLoc));
        DEBUG( std::cerr << "Source (" << startLoc.getRawEncoding() << ":" << endLoc.getRawEncoding() << "): "
              << source.substr(startLocSize, source.length() - startLocSize - endLocSize) << "\n"; );
        
        result << source.substr(startLocSize, source.length() - startLocSize - endLocSize);
      }
      // get range.first, range.second from our bookkeeping
      result << modifiedRanges_[ModifiedRange(range.first, range.second)];
      current = range.second;
      offsetStartLoc = true;
      DEBUG( std::cerr << "Result (" << range.first << ":" << range.second << "): " << modifiedRanges_[ModifiedRange(range.first, range.second)] << "\n"; );
    }
    
    // get rest from sources if we have not yet rendered until end
    if (current != rawEnd) {
      clang::SourceLocation startLoc = clang::SourceLocation::getFromRawEncoding(current);
      clang::SourceLocation endLoc = clang::SourceLocation::getFromRawEncoding(rawEnd);
      assert(offsetStartLoc);
      int startLocSize = rewriter_.getRangeSize(clang::SourceRange(startLoc, startLoc));
      std::string source = getOriginalText(clang::SourceRange(startLoc, endLoc));
      result << source.substr(startLocSize);
      DEBUG( std::cerr << "Result (" << startLoc.getRawEncoding() << ":" << endLoc.getRawEncoding() << "): " << source.substr(startLocSize) << "\n"; );
    }
    
    retVal = result.str();
  }
  
  std::cerr << "Get SourceLoc "
            << rawStart << ":" << rawEnd << " : " << retVal << "\n";
  
  return retVal;
}

void WebCLRewriter::applyTransformations()
{
  clang::tooling::Replacements replacements;

  // Go through modifications and replace them to source.
  ModificationMap &replacementMap = modifiedRanges();
  for (ModificationMap::iterator i = replacementMap.begin(); i != replacementMap.end(); ++i) {
    clang::SourceLocation startLoc = i->first.first;
    clang::SourceLocation endLoc = i->first.second;
    std::string replacement = i->second;
    replacements.insert(
        clang::tooling::Replacement(
            rewriter_.getSourceMgr(),
            clang::CharSourceRange::getTokenRange(startLoc, endLoc),
            replacement));
  }

  modifiedRanges_.clear();
  isFilteredRangesDirty_ = false;

  clang::tooling::applyAllReplacements(replacements, rewriter_);
}

WebCLRewriter::ModificationMap& WebCLRewriter::modifiedRanges()
{
  RangeModificationsFilter &rawModifications = filteredModifiedRanges();
  externalMap_.clear();
  for (RangeModificationsFilter::iterator i = rawModifications.begin();
       i != rawModifications.end(); i++) {
    
    ModifiedRange rawRange = *i;
    std::string& replacement = modifiedRanges_[rawRange];
    clang::SourceLocation start =
        clang::SourceLocation::getFromRawEncoding(rawRange.first);
    clang::SourceLocation end =
        clang::SourceLocation::getFromRawEncoding(rawRange.second);

    externalMap_[WclSourceRange(start, end)] = replacement;
  }

  return externalMap_;
}

WebCLRewriter::RangeModificationsFilter&
WebCLRewriter::filteredModifiedRanges() {
  
  // filter nested ranges out from added range modifications
  if (isFilteredRangesDirty_) {
    filteredModifiedRanges_.clear();
    
    int currentStart = -1;
    int biggestEnd = -1;
    
    // find out only toplevel ranges without nested areas
    for (RangeModifications::iterator i = modifiedRanges_.begin(); i != modifiedRanges_.end(); i++) {
      
      // DEBUG( std::cerr << "Range " << i->first.first << ":" << i->first.second << " = " << i->second << "\n"; );
      
      if (i->first.first < biggestEnd) {
        // DEBUG( std::cerr << "Skipping: " << "\n"; );
        continue;
      }
      
      if (currentStart != i->first.first) {
        if (currentStart != -1) {
          // This is a big vague because
          // area size might be zero in location ids
          // if current area cannot overlap earlier or if no earlier or it does not overlap => add area
          if (currentStart != biggestEnd ||
              filteredModifiedRanges_.empty() ||
              currentStart != filteredModifiedRanges_.rbegin()->second) {
            // DEBUG( std::cerr << "Added range: " << currentStart << ":" << biggestEnd << "\n"; );
            filteredModifiedRanges_.insert(ModifiedRange(currentStart, biggestEnd));
          }
        }
        currentStart = i->first.first;
      }
      
      biggestEnd = i->first.second;
    }
    
    // if last currentStart, biggestEnd is not nested... this is a bit nasty because source location start and end might be
    // the same which is a big ambigious...
    if (currentStart != biggestEnd ||
        filteredModifiedRanges_.empty() ||
        currentStart != filteredModifiedRanges_.rbegin()->second) {
      // DEBUG( std::cerr << "Added range: " << currentStart << ":" << biggestEnd << "\n"; );
      filteredModifiedRanges_.insert(ModifiedRange(currentStart, biggestEnd));
    }
  }
  isFilteredRangesDirty_ = false;
  
  return filteredModifiedRanges_;
}
