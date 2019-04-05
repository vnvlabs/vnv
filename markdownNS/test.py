import sys
import markdown
import markdownNS as mda

filess = [10,10,10]

extension_c = { "f" : filess } 


print mda.getMarkdown(sys.argv[1],extension_c)


