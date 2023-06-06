import sys
import os

with open(os.path.expanduser("~/.vnv"),'r+') as f:
    a = {i.strip() for i in f.readlines() if os.path.exists(i.strip()) }

    if sys.argv[1] == "register":
        a.add(sys.argv[2])
        
    elif sys.argv[1] == "deregister":
      if sys.argv[2] in a:
          a.remove(sys.argv[2])
      else:
          print(a,sys.argv[2])   
    elif sys.argv[1] == "clean":
        pass
    else:
        print("unknown argument ", sys.argv[1])
        exit(1)
    
    print(a)
    f.seek(0)
    f.write("\n".join(a))
    f.truncate()


    
    