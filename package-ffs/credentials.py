import os
import json
import uuid
import qrcode
from PIL import Image, ImageFont, ImageDraw

Import("env")

qr_dir = "./qr"
template = '''
{
  "rest-acc": "admin",
  "rest-pass": "admin",
  "host-name": "<template>",
  "default-config": {
    "wifi-mode": 2,
    "wifi-ap-ssid": "<template>",
    "wifi-ap-pass": "<template>",
    "config-restored": 0,
    "rest-acc": "admin",
    "rest-pass": "<template>"
  }
}
'''


def process_dir(path, hostname, ssid, passwd):
    with open(path, 'w') as json_file:
        data = json.loads(template)
        data["host-name"] = hostname
        data["rest-pass"] = passwd
        data["default-config"]["wifi-ap-ssid"] = ssid
        data["default-config"]["wifi-ap-pass"] = passwd
        data["default-config"]["rest-pass"] = passwd
        json.dump(data, json_file, indent=4, sort_keys=True)


def template_resources(project):
    SSID_SUFIX = uuid.uuid4().hex[-8:]
    HOSTNAME = "%s-%s" % (project, SSID_SUFIX)
    SSID = "%s-%s" % (project.split("-")[-1:][0], SSID_SUFIX)
    PWD = uuid.uuid4().hex[-8:]

    process_dir("./resources/json/config-global.json", HOSTNAME, SSID, PWD)
    img = qrcode.make('WIFI:S:%s;T:WPA;P:%s;;' % (SSID, PWD)).convert('RGB')

    padded_img = Image.new(
        "RGB", (img.width, img.height + 120), color=(255, 255, 255))
    padded_img.paste(img, (0, -5))

    font = ImageFont.truetype('../package-ffs/clacon.ttf', 35)

    d = ImageDraw.Draw(padded_img)
    d.text((40, img.height - 20), "SSID: %s" % SSID, fill=(0, 0, 0), font=font)
    d.text((40, img.height + 10), "Password: %s" %
           PWD, fill=(0, 0, 0), font=font)
    d.text((40, img.height + 40), "User: admin", fill=(0, 0, 0), font=font)
    d.text((40, img.height + 70), "Password: %s" %
           PWD, fill=(0, 0, 0), font=font)

    if not os.path.isdir(qr_dir):
        os.mkdir(qr_dir)

    padded_img.save("%s/qr-%s-%s.png" % (qr_dir, SSID, PWD))


if "uploadfs" in BUILD_TARGETS or "buildfs" in BUILD_TARGETS:
    if "__CREDENTIALS__" in [x[0] for x in env["CPPDEFINES"]]:
        template_resources(os.getcwd().split("/")[-1:][0].lower())
