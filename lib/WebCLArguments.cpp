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

#include "WebCLArguments.hpp"
#include "WebCLConfiguration.hpp"
#include "kernel.h"

#include <algorithm>
#include <cassert>
#include <cstdio>
#include <cstdlib>
#include <fstream>
#include <iostream>
#include <set>
#include <fcntl.h>

#ifdef _MSC_VER
#include <io.h>
#else
#include <unistd.h>
#endif

// IMPROVEMENT: Maybe we should resolve directory separator and temp directory
//              in windows runtime, since it depends if we are running from msys / cygwin shell
//              or from windows commandline?

#if (defined(__MINGW32__) || defined(_MSC_VER))
#define DIR_SEPARATOR "\\"
#define TEMP_DIR "."
#else
#define DIR_SEPARATOR "/"
#define TEMP_DIR P_tmpdir
#endif

int mingwcompatible_mkstemp(char* tmplt) {
  char *filename = mktemp(tmplt);
  if (filename == NULL) return -1;
  return open(filename, O_RDWR | O_CREAT, 0600);
}

namespace {
    // apparently wcl_strdup isn't in C standard, only in POSIX. Also we get to use 'delete[]' on these strings.
    char* wcl_strdup(const char* str)
    {
        if (!str) {
            return 0;
        } else {
            size_t storage_len = strlen(str) + 1;
            char* clone = new char[storage_len];
            std::copy(str, str + storage_len, clone);
            return clone;
        }
    }
}

WebCLArguments::WebCLArguments(const std::string &inputSource, const CharPtrVector& argv)
    : preprocessorArgv_()
    , validatorArgv_()
    , files_()
    , builtinDeclFilename_(NULL)
    , outputs_()
{
    int inputDescriptor = -1;
    char const *inputFilename = createFullTemporaryFile(inputDescriptor, &inputSource[0], inputSource.size());
    if (!inputFilename)
        return;
    files_.push_back(TemporaryFile(-1, inputFilename));
    close(inputDescriptor);

    char const *buffer = reinterpret_cast<char const*>(kernel_endlfix_cl);
    size_t length = kernel_endlfix_cl_len;
    int headerDescriptor = -1;
    char const *headerFilename = createFullTemporaryFile(headerDescriptor, buffer, length);
    if (!headerFilename)
        return;
    files_.push_back(TemporaryFile(-1, headerFilename));
    close(headerDescriptor);

    int builtinDeclDescriptor = -1;
    builtinDeclFilename_ = createEmptyTemporaryFile(builtinDeclDescriptor);
    if (!builtinDeclFilename_)
        return;
    files_.push_back(TemporaryFile(-1, builtinDeclFilename_));
    close(builtinDeclDescriptor);

    // TODO: add -Dcl_khr_fp16 etc definitions to preprocessor options
    // based on extensions passed to clvValidate()

    char const *findUsedExtensionsInvocation[] = {
        "libclv", inputFilename, "--",
        "-include", headerFilename
    };
    const int findUsedExtensionsInvocationSize =
        sizeof(findUsedExtensionsInvocation) / sizeof(findUsedExtensionsInvocation[0]);

    char const *findUsedExtensionsOptions[] = {
        "-E", "-x", "cl", "-fno-builtin", "-ffreestanding",
        "-D_W2CL_EXTENSION_ALL"
    };
    const int numFindUsedExtensionsOptions =
        sizeof(findUsedExtensionsOptions) / sizeof(findUsedExtensionsOptions[0]);

    char const *preprocessorOptions[] = {
        "-E", "-x", "cl", "-fno-builtin", "-ffreestanding"
    };
    const int numPreprocessorOptions =
        sizeof(preprocessorOptions) / sizeof(preprocessorOptions[0]);
    char const *preprocessorInvocation[] = {
        "libclv", inputFilename, "--"
    };
    const int preprocessorInvocationSize =
        sizeof(preprocessorInvocation) / sizeof(preprocessorInvocation[0]);

    char const *validatorInvocation[] = {
        "libclv", NULL, "--"
    };
    const int validatorInvocationSize =
        sizeof(validatorInvocation) / sizeof(validatorInvocation[0]);
    char const *validatorOptions[] = {
        "-x", "cl",
        "-include", headerFilename, // has to be early (provides utility macros)
        "-include", builtinDeclFilename_ // has to be late (uses utility macros)
    };
    const int numValidatorOptions =
        sizeof(validatorOptions) / sizeof(validatorOptions[0]);

    std::set<char const *> userDefines;
    for (size_t i = 0; i < argv.size(); ++i) {
        char const *option = argv[i];
        if (!std::string(option).substr(0, 2).compare("-D"))
            userDefines.insert(option);
    }

    // find used extensions arguments
    std::transform(findUsedExtensionsInvocation,
        findUsedExtensionsInvocation + findUsedExtensionsInvocationSize,
        std::back_inserter(findUsedExtensionsArgv_),
        wcl_strdup);
    std::transform(userDefines.begin(),
        userDefines.end(),
        std::back_inserter(findUsedExtensionsArgv_),
        wcl_strdup);
    std::transform(findUsedExtensionsOptions,
        findUsedExtensionsOptions + numFindUsedExtensionsOptions,
        std::back_inserter(findUsedExtensionsArgv_),
        wcl_strdup);
    findUsedExtensionsArgv_.push_back(wcl_strdup("-ferror-limit=0"));

    // preprocessor arguments
    std::transform(preprocessorInvocation,
        preprocessorInvocation + preprocessorInvocationSize,
        std::back_inserter(preprocessorArgv_),
        wcl_strdup);
    std::transform(userDefines.begin(),
        userDefines.end(),
        std::back_inserter(preprocessorArgv_),
        wcl_strdup);
    std::transform(preprocessorOptions,
        preprocessorOptions + numPreprocessorOptions,
        std::back_inserter(preprocessorArgv_),
        wcl_strdup);
    preprocessorArgv_.push_back(wcl_strdup("-ferror-limit=0"));

    // validator arguments
    std::transform(validatorInvocation,
        validatorInvocation + validatorInvocationSize,
        std::back_inserter(validatorArgv_),
        wcl_strdup);
    std::transform(argv.begin(),
        argv.end(),
        std::back_inserter(validatorArgv_),
        wcl_strdup);
    std::transform(validatorOptions,
        validatorOptions + numValidatorOptions,
        std::back_inserter(validatorArgv_),
        wcl_strdup);
    validatorArgv_.push_back(wcl_strdup("-ferror-limit=0"));
    validatorArgv_.push_back(wcl_strdup("-fno-builtin"));
    validatorArgv_.push_back(wcl_strdup("-ffreestanding"));
}

WebCLArguments::~WebCLArguments()
{
    for (size_t i = 0; i < findUsedExtensionsArgv_.size(); ++i) {
        delete[] findUsedExtensionsArgv_[i];
    }

    for (size_t i = 0; i < preprocessorArgv_.size(); ++i) {
        delete[] preprocessorArgv_[i];
    }

    for (size_t i = 0; i < validatorArgv_.size(); ++i) {
        delete[] validatorArgv_[i];
    }

    while (files_.size()) {
        TemporaryFile &file = files_.back();
        // close(file.first);
        remove(file.second);
        delete[] file.second;
        files_.pop_back();
    }
}

CharPtrVector WebCLArguments::getFindUsedExtensionsArgv() const
{
    return findUsedExtensionsArgv_;
}

CharPtrVector WebCLArguments::getPreprocessorArgv() const
{
    if (!preprocessorArgv_.size())
        return CharPtrVector();
    // Input file has been already set.
    return preprocessorArgv_;
}

CharPtrVector WebCLArguments::getMatcherArgv()
{
    if (!validatorArgv_.size())
        return CharPtrVector();

    CharPtrVector matcherArgv(validatorArgv_);

    // Set input file.
    char const *input = outputs_.back();
    matcherArgv[1] = input;

    return matcherArgv;
}

CharPtrVector WebCLArguments::getValidatorArgv() const
{
    if (!validatorArgv_.size())
        return CharPtrVector();

    // Set input file.
    CharPtrVector argv(validatorArgv_);
    char const *input = outputs_.back();
    argv[1] = input;

    return argv;
}

char const *WebCLArguments::getInput(const CharPtrVector& argv)
{
    if (!argv.size())
        return NULL;

    return argv[1];
}

void WebCLArguments::createOutput()
{
    int fd = -1;
    char const *name = createEmptyTemporaryFile(fd);
    if (!name)
        return;
    files_.push_back(TemporaryFile(-1, name));
    close(fd);
    outputs_.push_back(name);
}

bool WebCLArguments::supplyBuiltinDecls(const std::string &decls)
{
    std::ofstream ofs(builtinDeclFilename_, std::ios::binary | std::ios::trunc);
    return ofs.good() && ofs << decls;
}

bool WebCLArguments::supplyExtensionArguments(const std::set<std::string> &extensions)
{
    WebCLConfiguration cfg;
    for (std::set<std::string>::const_iterator it = extensions.begin();
         it != extensions.end();
         ++it) {
        validatorArgv_.push_back(wcl_strdup(("-D" + cfg.getExtensionDefineName(*it)).c_str()));
        preprocessorArgv_.push_back(wcl_strdup(("-D" + cfg.getExtensionDefineName(*it)).c_str()));
    }
    return true;
}

bool WebCLArguments::areArgumentsOk(int argc, char const **argv) const
{
    return (argc > 1) && argv;
}

char const *WebCLArguments::createEmptyTemporaryFile(int &fd) const
{
    char const *directory = TEMP_DIR;
    char const *filename = DIR_SEPARATOR "wclXXXXXX";

    const int directoryLength = strlen(directory);
    const int filenameLength = strlen(filename);
    const int templateLength = directoryLength + filenameLength;

    char *result = new char[templateLength + 1];
    if (!result) {
        std::cerr << "Internal error. Can't create temporary filename." << std::endl;
        return NULL;
    }
 
    std::copy(directory, directory + directoryLength, result);
    std::copy(filename, filename + filenameLength, result + directoryLength);
    result[templateLength] = '\0';

    fd = mingwcompatible_mkstemp(result);
    if (fd == -1) {
        std::cerr << "Internal error. Can't create temporary file." << std::endl;
        delete[] result;
        return NULL;
    }

    return result;
}

char const *WebCLArguments::createCopiedTemporaryFile(int srcFd) const
{
    int dstFd;
    char const *filename = createEmptyTemporaryFile(dstFd);
    if (!filename)
        return NULL;

    char buffer[1024];
    ssize_t rdBytes;
    while ((rdBytes = read(srcFd, buffer, sizeof(buffer))) > 0) {
	const size_t wrBytes = write(dstFd, buffer, rdBytes);
	if (wrBytes != size_t(rdBytes)) {
	    std::cerr << "Internal error. Can't populate temporary file." << std::endl;
	    delete[] filename;
	    close(dstFd);
	    return NULL;
	}
    }
    close(dstFd);
    return filename;
}

char const *WebCLArguments::createFullTemporaryFile(int &fd, char const *buffer, size_t length) const
{
    char const *filename = createEmptyTemporaryFile(fd);
    if (!filename)
        return NULL;

    const size_t bytes = write(fd, buffer, length);
    if (bytes != length) {
        std::cerr << "Internal error. Can't populate temporary file." << std::endl;
        delete[] filename;
        close(fd);
        return NULL;
    }

    return filename;
}
