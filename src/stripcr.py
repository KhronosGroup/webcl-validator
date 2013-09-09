# git on windows might mess up line endings 
# added separate script because sed on osx does not work for removing CRs
import sys
print open(sys.argv[1]).read().replace('\r','')