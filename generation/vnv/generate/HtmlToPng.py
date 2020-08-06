from selenium import webdriver
import os
import shutil
import tempfile
import pathlib
from PIL import Image
import numpy as np
import uuid
from skimage.io import imread
from skimage.morphology import convex_hull_image
from skimage.color import rgb2gray
from .configs import getApp

chrome_driver = None
chrome_avail = True

# This function uses the chrome driver and seleium to take a screen shot
# of the given url. This is used by the custom extensions to generate images
# for javascript only applications.
generic_html = '''
<!doctype html>

<html lang="en">
<head>
  <meta charset="utf-8">
  <title>VnV</title>
  <meta name="VnV" content="Generic">
  <meta name="VnV" content="SitePoint">
  {scripts}
</head>

<body>
 {body}
</body>
</html>
'''


def getGeneric(scripts, body):
    return generic_html.format(scripts=scripts, body=body)


iframeTemplate = '''
 <iframe id='{idr}' class='resizer' src="{srcfile}"  frameborder='0' border='0' cellspacing='0' style='border-style:none; width:1px; min-width:100%;'></iframe>
 <script> iFrameResize({{}},'#{idr}'); </script>
'''

iframe_count = 0


def getIFrame(html, header="", width="100%"):

    global iframe_count

    outdir = os.path.join(getApp().outdir, "_static", "js", "gen")
    if not os.path.exists(outdir):
        os.makedirs(outdir)

    # Once in a billion years this might cause an issue
    id_ = "ifid-{}".format(iframe_count)
    iframe_count += 1

    srcfilename = "_static/js/gen/" + uuid.uuid4().hex + ".html"
    htmlfilename = "/" + srcfilename

    with open(os.path.join(getApp().outdir, srcfilename), 'w') as f:
        f.write(getGeneric(scripts=header, body=html))

    return iframeTemplate.format(srcfile=htmlfilename,
                                 width=width,
                                 idr=id_)


driver = None


def smart_crop(filename):
    im = imread(filename)
    # create a binary image
    im1 = 1 - rgb2gray(im)
    threshold = 0.5
    im1[im1 <= threshold] = 0
    im1[im1 > threshold] = 1
    chull = convex_hull_image(im1)
    imageBox = Image.fromarray((chull * 255).astype(np.uint8)).getbbox()
    cropped = Image.fromarray(im).crop(imageBox)
    cropped.save(filename)


def generateImage(scripts, body, outfileName):
    try:
        if not chrome_avail:
            return False
        elif chrome_driver is None:
            driver = webdriver.Chrome()
        else:
            driver = webdriver.Chrome(chrome_driver)
        print("SDFSDFD")
        print(getGeneric(scripts, body))
        with open(outfileName + ".html", 'w') as f:
            f.write(getGeneric(scripts, body))

        print(os.path.abspath(outfileName + ".html"))
        driver.get(
            pathlib.Path(
                os.path.abspath(
                    outfileName +
                    ".html")).as_uri())
        screenshot = driver.save_screenshot(outfileName + ".png")
        driver.quit()
        smart_crop(outfileName + ".png")
        return True
    except Exception as e:
        print(e)
        return False


def getScript(src):
    return '''<script src="{src}"/>'''.format(src)


if __name__ == "__main__":
    scripts = ""
    body = '''<div>HGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGG</div>'''
    generateImage(scripts, body, "./testImage")
