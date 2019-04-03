import sys
import markdown
from markdownNS import MarkdownNS

print markdown.markdown(sys.argv[1], extensions=[MarkdownNS()])

