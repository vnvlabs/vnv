import sys
import markdown
from markdownNS import MarkdownNS

filess = [10,10,10]

extension_c = { "f" : filess } 

print markdown.markdown(sys.argv[1], extensions=[MarkdownNS(extension_c)])

