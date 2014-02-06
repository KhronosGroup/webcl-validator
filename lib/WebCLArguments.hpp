#ifndef WEBCLVALIDATOR_WEBCLARGUMENTS
#define WEBCLVALIDATOR_WEBCLARGUMENTS

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

#include <cstring>
#include <string>
#include <utility>
#include <vector>
#include <set>

#include "WebCLCommon.hpp"

/// Chains tools performing different validation stages. Links outputs
/// of earlier stages to inputs of later stages. Makes sure that tool
/// arguments match the pipeline.
///
/// \see WebCLTool
class WebCLArguments
{
public:

    /// Constructor. The command line options given by user should be
    /// passed as arguments, along with the contents of the input file.
    WebCLArguments(const std::string &inputSource, const CharPtrVector& argv);
    ~WebCLArguments();

    /// \return Tool arguments for the phase finding used extensions.
    /// Input is the original input.
    CharPtrVector getFindUsedExtensionsArgv() const;

    /// \return Preprocessor tool arguments.
    CharPtrVector getPreprocessorArgv() const;

    /// \return Normalization tool arguments. Input is matched with
    /// output of previous (preprocessor or normalization) tool.
    CharPtrVector getMatcherArgv();

    /// \return Memory access validation tool arguments. Input is
    /// matched with output of previous normalization tool.
    CharPtrVector getValidatorArgv() const;

    /// \return Input file for a tool with the given
    /// options.
    char const *getInput(const CharPtrVector& argv);

    /// Creates an output file that becomes the input file of the next tool in
    /// the pipeline.
    void createOutput();

    /// Writes the given data into a file that is included as an input
    /// by the matcher and validator tools due to their argv's
    /// \return \c true on success, \c false on failure
    bool supplyBuiltinDecls(const std::string &decls);

    /// Writes the given data into a file that is included as an input
    /// by the matcher and validator tools due to their argv's
    /// \return \c true on success, \c false on failure
    bool supplyExtensionArguments(const std::set<std::string> &extensions);

private:

    /// Whether there is room for input file.
    bool areArgumentsOk(int argc, char const **argv) const;

    /// \return Empty file that will be used for tool output.
    char const *createEmptyTemporaryFile(int &fd) const;
    /// \return File that has been initialized from a buffer. Allows
    /// tool to use '-include' option to include contents that have
    /// been embedded into main executable.
    char const *createFullTemporaryFile(int &fd, char const *buffer, size_t length) const;
    /// \return File that has been initialized from the contents
    /// of srcFd (typically standard input)
    char const *createCopiedTemporaryFile(int srcFd) const;

    /// Preprocessor arguments.
    CharPtrVector findUsedExtensionsArgv_;
    /// Preprocessor arguments.
    CharPtrVector preprocessorArgv_;
    /// Arguments for normalization and memory access validation.
    CharPtrVector validatorArgv_;

    /// Pair of file descriptor and filename.
    typedef std::pair<int, char const *> TemporaryFile;
    /// Save information about temporary files so that we can close
    /// and remove them. Contains generated headers and output files.
    typedef std::vector<TemporaryFile> TemporaryFiles;
    TemporaryFiles files_;

    /// Contains the name of a temporary header used to include
    /// select builtin function declarations in matcher and validation stages
    char const *builtinDeclFilename_;

    /// Save information about output files so that we can chain
    /// different tools properly. Contains only those files that are
    /// both outputs and inputs of adjacent tools.
    typedef std::vector<char const *> OutputFiles;
    OutputFiles outputs_;
};

#endif // WEBCLVALIDATOR_WEBCLARGUMENTS
