from Screenshot import Screenshot_Clipping
from selenium import webdriver

''' Eventually this will be used to screenshot html files
    to generate images for latex docs. I came across this
    code somewhere else, so thought I would drop it here
    for later use. Basically, pass in the url of the page
    you are looking for, and the ID of the element to screenshot,
    and it will produce a png of it. '''


class ImageCreator:

    driver = None
    ob = None

    def __init__(self):
        self.driver = webdriver.Chrome()
        self.ob = Screenshot_Clipping.Screenshot()

    def __del__(self):
        self.driver.quit()

    def getScreenShot(self, url, componentId, outDir, outName):
        self.driver.get(url)
        element = self.driver.find_element_by_id(componentId)
        img_url = self.ob.get_element(
            self.driver,
            element,
            save_path=outDir,
            image_name=outName)
        print(img_url)
        self.driver.close()


image_creator = ImageCreator()
