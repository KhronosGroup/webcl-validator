# git on windows might mess up line endings 
# added separate script because sed on osx does not work for removing CRs
import sys
open(sys.argv[2], "wb+").write(open(sys.argv[1],"rb").read().replace('\r',''))