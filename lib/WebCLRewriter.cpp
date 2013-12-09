/*
** Copyright (c) 2013 The Khronos Group Inc.
**
** Permission is hereby granted, free of charge, to any person obtaining a
** copy of this software and/or associated documentation files (the
** "Materials"), to deal in the Materials without restriction, including
** without limitation the rights to use, copy, modify, merge, publish,
** distribute, sublicense, and/or sell copies of the Materials, and to
** permit persons to whom the Materials are furnished to do so, subject to
** the following conditions:
**
** The above copyright notice and this permission notice shall be included
** in all copies or substantial portions of the Materials.
**
** THE MATERIALS ARE PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
** EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
** MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
** IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY
** CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
** TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
** MATERIALS OR THE USE OR OTHER DEALINGS IN THE MATERIALS.
*/

#include "WebCLDebug.hpp"
#include "WebCLRewriter.hpp"

#include "clang/Frontend/CompilerInstance.h"
#include "clang/Lex/Lexer.h"
#include "clang/Rewrite/Core/Rewriter.h"
#include "clang/Tooling/Refactoring.h"
#include "clang/AST/Expr.h"

#include <sstream>
#include <cctype>

WebCLRewriter::WebCLRewriter(clang::CompilerInstance &instance,
                             clang::Rewriter &rewriter)
    : instance_(instance)
    , rewriter_(rewriter)
    , isFilteredRangesDirty_(false)
{
}

namespace {
  // Finds the location of a certain character
  class CharacterIteratorBase {
  public:
    CharacterIteratorBase() {}
    virtual ~CharacterIteratorBase() {}

    virtual bool operator()(clang::SourceLocation currentLocation, char currentChar) = 0;
  };

  class CharacterFinder: public CharacterIteratorBase {
  public:
    CharacterFinder(char charToFind) : charToFind_(charToFind) {}

    bool operator()(clang::SourceLocation, char currentChar) {
      return currentChar != charToFind_;
    }

  private:
    char charToFind_;
  };

  // Finds function expression and argument expression source locations
  class ArgumentFinder: public CharacterIteratorBase {
  private:
    enum State {
      STATE_WAIT_FUNC_NAME,	     // waiting function name to begin
      STATE_INSIDE_FUNC_NAME,	     // inside a function name (not inside parens)
      STATE_INSIDE_FUNC_NAME_PARENS, // inside a function (an expression with parens)
      STATE_WAIT_FUNC_ARGS,	     // waiting function arguments to begin
      STATE_INSIDE_FUNC_ARGS,	     // inside function arguments
      STATE_INSIDE_FUNC_ARGS_PARENS, // inside a function argument containing parens
      STATE_ARGS_FINISHED,	     // finished

      STATE_ERROR_MASK                = 0xf0,
      STATE_ERROR_STRINGS_UNSUPPORTED = 0x10 // strings not supported at this time..
    };

    enum CommentState {
      CSTATE_NONE,
      CSTATE_ENTER_MAYBE,
      CSTATE_LINE,		// a single-line comment
      CSTATE_COMMENT,		// a long comment
      CSTATE_COMMENT_EXIT_MAYBE // we found an asterisk, maybe exit comment
    };

  public:
    ArgumentFinder();

    bool operator()(clang::SourceLocation location, char currentChar);

    bool hasError() const;
    bool hasWorkLeft() const;

    // may contain parens etc, for example (foo)(1, 2)
    clang::SourceRange functionName_;

    // contains preceding and following whitespace if exists
    WebCLRewriter::SourceRangeVector arguments_;

  private:
    // handles a charcater without dealing with comments or strings
    void handleChar(clang::SourceLocation location, char currentChar);

    State state_;		         /// current state
    CommentState cstate_;		 /// currently inside a line comment?
    int openParens_;                     /// number of open parenthesis in current state
    clang::SourceLocation lastLocation_; /// last interesting location, ie. beginning on an argument

    char previousChar_;			     /// previous char when we notice it wasn't a comment
    clang::SourceLocation previousLocation_; /// previous location -"-
  };

  ArgumentFinder::ArgumentFinder()
    : state_(STATE_WAIT_FUNC_NAME)
    , cstate_(CSTATE_NONE)
    , openParens_(0)
  {
    // nothing
  }

  bool ArgumentFinder::hasError() const
  {
    return state_ & STATE_ERROR_MASK;
  }

  bool ArgumentFinder::hasWorkLeft() const
  {
    return state_ != STATE_ARGS_FINISHED && !hasError();
  }
  
  void ArgumentFinder::handleChar(clang::SourceLocation currentLocation, char currentChar) {
    switch (state_) {
    case STATE_ARGS_FINISHED: 
    case STATE_ERROR_MASK: 
    case STATE_ERROR_STRINGS_UNSUPPORTED: {
      // remove compiler warnings
    } break;
    case STATE_WAIT_FUNC_NAME: {
      switch (currentChar) {
      case '(': {
	lastLocation_ = currentLocation;
	state_ = STATE_INSIDE_FUNC_NAME_PARENS;
	++openParens_;
      } break;
      default:
	if (isalpha(currentChar)) {
	  lastLocation_ = currentLocation;
	  state_ = STATE_INSIDE_FUNC_NAME;
	}
      }
    } break;
    case STATE_INSIDE_FUNC_NAME: {
      switch (currentChar) {
      case '(': {
	functionName_ = clang::SourceRange(lastLocation_, currentLocation.getLocWithOffset(-1));
	lastLocation_ = currentLocation.getLocWithOffset(1);
	state_ = STATE_INSIDE_FUNC_ARGS;
      } break;
      }
    } break;
    case STATE_INSIDE_FUNC_NAME_PARENS: {
      switch (currentChar) {
      case '(': {
	++openParens_;
      } break;
      case ')': {
	if (--openParens_ == 0) {
	  functionName_ = clang::SourceRange(lastLocation_, currentLocation.getLocWithOffset(-1));
	  state_ = STATE_WAIT_FUNC_ARGS;
	}
      } break;
      }
    } break;
    case STATE_WAIT_FUNC_ARGS: {
      switch (currentChar) {
      case '(': {
	lastLocation_ = currentLocation.getLocWithOffset(1);
	state_ = STATE_INSIDE_FUNC_ARGS;
      } break;
      }
    } break;
    case STATE_INSIDE_FUNC_ARGS: {
      switch (currentChar) {
      case '(': {
	state_ = STATE_INSIDE_FUNC_ARGS_PARENS;
	++openParens_;
      } break;
      case ',': {
	arguments_.push_back(clang::SourceRange(lastLocation_, currentLocation.getLocWithOffset(-1)));
	lastLocation_ = currentLocation.getLocWithOffset(1);
	// stay in the same state_
      } break;
      case ')': {
	arguments_.push_back(clang::SourceRange(lastLocation_, currentLocation.getLocWithOffset(-1)));
	state_ = STATE_ARGS_FINISHED;
      } break;
      }
    } break;
    case STATE_INSIDE_FUNC_ARGS_PARENS: {
      switch (currentChar) {
      case '(': {
	++openParens_;
      } break;
      case ')': {
	if (--openParens_ == 0) {
	  state_ = STATE_INSIDE_FUNC_ARGS;
	}
      } break;
      }
    } break;
    }
  }

  bool ArgumentFinder::operator()(clang::SourceLocation currentLocation, char currentChar) {
    // the only way to read this is to go through the strings foo(bar, baz) and (foo)(bar, baz)
    if (currentChar == '"') {
      state_ = STATE_ERROR_STRINGS_UNSUPPORTED;
    } else if (cstate_ == CSTATE_NONE && currentChar == '/') {
      previousLocation_ = currentLocation;
      previousChar_ = currentChar;
      cstate_ = CSTATE_ENTER_MAYBE;
    } else if (cstate_ == CSTATE_ENTER_MAYBE && currentChar == '/') {
      cstate_ = CSTATE_LINE;
    } else if (cstate_ == CSTATE_LINE && currentChar == '\n') {
      cstate_ = CSTATE_NONE;
    } else if (cstate_ == CSTATE_ENTER_MAYBE && currentChar == '*') {
      cstate_ = CSTATE_COMMENT;
    } else if (cstate_ == CSTATE_COMMENT && currentChar == '*') {
      cstate_ = CSTATE_COMMENT_EXIT_MAYBE;
    } else if (cstate_ == CSTATE_COMMENT_EXIT_MAYBE && currentChar == '/') {
      cstate_ = CSTATE_NONE;
    } else if (cstate_ == CSTATE_COMMENT_EXIT_MAYBE) {
      cstate_ = CSTATE_COMMENT;
    } else if (cstate_ == CSTATE_ENTER_MAYBE) {
      handleChar(previousLocation_, previousChar_);
      handleChar(currentLocation, currentChar);
    } else {
      handleChar(currentLocation, currentChar);
    }
    return hasWorkLeft();
  }


  // Iterate a source file forward by applying fn on each character until fn
  // returns false. Return the position where false was returned. Fn may
  // perform side effects to keep track of its internal state.
  clang::SourceLocation iterateSourceWhile(clang::CompilerInstance &instance, clang::SourceLocation startLoc, CharacterIteratorBase &fn) {
    // if we could use matchers to find these locations would be better
    const clang::SourceManager &SM = instance.getSourceManager();
    std::pair<clang::FileID, unsigned> locInfo = SM.getDecomposedLoc(startLoc);
    bool invalidTemp = false;
    clang::StringRef file = SM.getBufferData(locInfo.first, &invalidTemp);
    assert(invalidTemp == false);
    const char *tokenBegin = file.data() + locInfo.second;
  
    clang::Lexer lexer(SM.getLocForStartOfFile(locInfo.first), instance.getLangOpts(),
      file.begin(), tokenBegin, file.end());
  
    // just raw find for next semicolon and get SourceLocation
    const char* endOfFile = SM.getCharacterData(SM.getLocForEndOfFile(locInfo.first));
    const char* charPos = SM.getCharacterData(startLoc);
  
    while (fn(lexer.getSourceLocation(charPos), *charPos) && charPos < endOfFile) {
      clang::SourceLocation currLoc = lexer.getSourceLocation(charPos);
      int currLength = clang::Lexer::MeasureTokenLength(currLoc, SM, instance.getLangOpts());
      // at least advance one char
      if (currLength == 0) {
	currLength = 1;
      }
      charPos += currLength;
      // std::cerr << "Curr token:" << rewriter_.getRewrittenText(clang::SourceRange(currLoc, currLoc)) << "\n";
    };
  
    return lexer.getSourceLocation(charPos);
  }
}

/// \brief Finds source location of next token which starts with given char
clang::SourceLocation WebCLRewriter::findLocForNext(clang::SourceLocation startLoc, char charToFind) {
  CharacterFinder finder(charToFind);
  return iterateSourceWhile(instance_, startLoc, finder);
}

WebCLRewriter::SourceRangeVector WebCLRewriter::getArgumentSourceRanges(clang::CallExpr *call)
{
  ArgumentFinder finder;
  iterateSourceWhile(instance_, call->getLocStart(), finder);

  if (finder.hasError()) {
    DEBUG (std::cerr << "ArgumentFinder failed" << std::endl; );
    return finder.arguments_;
  } else {
    DEBUG (
      std::cerr << "original text: " << getOriginalText(call->getSourceRange()) << std::endl;;

      SourceRangeVector ranges = finder.arguments_;

      for (SourceRangeVector::const_iterator it = ranges.begin();
	   it != ranges.end();
	   ++it) {
	std::cerr << "argument: " << getOriginalText(*it) << std::endl;
      }
    );

    return finder.arguments_;
  }
}

void WebCLRewriter::removeText(clang::SourceRange range) {
  isFilteredRangesDirty_ = true;
  DEBUG( std::cerr << "Remove SourceLoc " << range.getBegin().getRawEncoding() << ":" << range.getEnd().getRawEncoding() << " " << getOriginalText(range) << "\n"; );
  replaceText(range, "");
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
    
    // LAUNDRY: if you are going to touch this code, refactor to use 
    //          filteredModifiedRanges() like flushing does
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
          // This is a bit vague because
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
  
  DEBUG( std::cerr << "Get SourceLoc "
         << rawStart << ":" << rawEnd << " : " << retVal << "\n"; );
  
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
