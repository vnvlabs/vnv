class VnVTreeGenerator:
    def getRestructuredText(self, node):
        return self.dispatcher(node)

    def dispatcher(self, node):
        print(node)
        return "Hello"

    def visitRootNode(self, node):
        pass

    def visitInjectionPointNode(self, node):
        pass

    def visitTestNode(self, node):
        pass

    def visitUnitTestNode(self, node):
        pass

    def visitUnitTestsNode(self, node):
        pass

    def visitLogNode(self, node):
        pass
