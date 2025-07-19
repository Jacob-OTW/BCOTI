# BCOTI (Budget-Clip-On-Thermal-Imager)
This project is about building a Clip-On-Thermal-Imager (COTI) for Analog night vision goggles, similar to the Jerry-C, PAS29A/B, etc. yourself.

The current version uses a 256x192@12um sensor with a 9.1mm lens. This combined with the LZE039 and 25mm DCX lens, gives you a 1x 17°x13° thermal overlay in your NVGs. At the moment the image is cropped to a 13°x13° Rectangle.

Cropping it to a 13° circle is also an option by modifying the shape of the `Display_Spacer.step` model.

The unit is powered by a standard USB-C power source delivering 5v.

# User Manual

## Setting image settings

The BCOTI uses the Wi-Fi interface of the esp32 board to act as a web server to host a basic WebUI. To start the unit with Wi-Fi enabled, 

1. hold down the push-button and then supply the unit with power, so either plug in your cable (given the poti is already enabled) or enable the poti, keep the button held down for a short moment, then let go.

2. Open your phones Wi-Fi settings and connect to "BCOTI", the password is "wifipass1234"

3. Inside a web browser, open "192.168.4.1".

4. At the top you select the active preset, so the one being both used by the unit in real time, and the one you make changes to.

5. You can either enable or disable a preset, this determines if the preset will be used, or skipped when using the push button on the unit.

6. Once you made all your changes, reboot the unit to run it on the normal mode. Then use the push button to cycle through your active presets.

### Recommended Settings
- Burn protection. This will automatically close the shutter of the thermal camera if you look at something very hot, like a fire, or the sun. Note that when the unit is not powered, the shutter is in the open position, so it's high recommended you cover the lens when the unit is not in use. Very hot object can burn the actual sensor.
- Auto Shutter. This will do Non-Uni

# Bill-of-materials
Expect to pay about 400€ (EU) or about 500usd (US) for parts when buying in small quantities

| Part                           | Cost   | Link                                                                                                   | Comment                                                                                                                                                                                                                                                                                                                                                                                                                                            |
|--------------------------------|--------|--------------------------------------------------------------------------------------------------------|----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------|
| Mini2                          | 189€   | [AliExpress (256 9.1mm option)](https://www.aliexpress.us/item/1005007938410808.html)                  | Only the 256 w/ 9.1mm version will work, others won't be 1x with current setup. [Hdaniee Store page incase the link doesn't work](https://www.aliexpress.com/store/1104212749?spm=a2g0o.detail.0.0.5c38Uf2FUf2Fd3), CV256 9.1mm. [For Some it might also be much cheaper to buy on Alibaba](https://www.alibaba.com/product-detail/HDANIEE-Series-Thermal-Imaging-Camera-Module_1601290252342.html?spm=a27aq.27095423.1978240560.1.78372277Z3cl82) |
| LZE039 OLED                    | 98€    | [AliExpress (AV 3color option)](https://www.aliexpress.us/item/1005008087046449.html)                  | Bare screen + AV driver.                                                                                                                                                                                                                                                                                                                                                                                                                           |
| Esp32c3 Super Mini             | 2.6€   | [AliExpress](https://www.aliexpress.us/item/1005007663345442.html)                                     | They can break during assembly, I'd recommend a 5pc for safety.                                                                                                                                                                                                                                                                                                                                                                                    |
| USB-C Connector Breakout (1x+) | 2.25€  | [AliExpress 2pc ("TYPE-C male 4P" option)](https://www.aliexpress.us/item/1005005870466748.html)       | At least 1x for device internal, but can also be used to created really low profile cable for helmet setup.                                                                                                                                                                                                                                                                                                                                        |
| USB-C Port                     | 1.49€  | [AliExpress ("4P Black" option)](https://www.aliexpress.us/item/1005005768848819.html)                 |                                                                                                                                                                                                                                                                                                                                                                                                                                                    |
| Toggle Switch                  | 1.19€  | [AliExpress ("3P 11.7ON-OFF-ON R" option)](https://www.aliexpress.us/item/1005008165436395.html)       |                                                                                                                                                                                                                                                                                                                                                                                                                                                    |
| Poti w/ ON-OFF-switch          | 2.59€  | [AliExpress 5pc ("20K Ohm" option)](https://aliexpress.us/item/1005003523748492.html)                  | Can also be higher resistance value, for voltage divider, without current draw like here, it doesn't really matter                                                                                                                                                                                                                                                                                                                                 |
| M2x4 Screws (11x)              | 1.35€  | [AliExpress 50pc (M2 x 50pc, 4mm length option)](https://www.aliexpress.us/item/1005008314123679.html) | I didn't use these exact ones, but should work, otherwise I adjust the hole size for the screw heads                                                                                                                                                                                                                                                                                                                                               |
| M2x3 Heat Inserts (11x)        | 9.99€  | [CNCKitchen 100pc](https://cnckitchen.store/products/heat-set-insert-m2-x-3-100-pieces)                | Could also get cheaper ones in similar size, but I like these because they give good info about the required hole sizes. Should you get one with a smaller OD, the FreeCad file has a paramter for that.                                                                                                                                                                                                                                           |
| 5mmD 25mmF DCX lens            | 41.61$ | [Edmund Optics](https://www.edmundoptics.com/p/5mm-dia-x-25mm-fl-uncoated-double-convex-lens/18171/)   | Price is really bad, but haven't found alternative yet.                                                                                                                                                                                                                                                                                                                                                                                            |
| 6x8.5mm surface mirror         | 27.03$ | [Edmund Optics](https://www.edmundoptics.com/p/6-x-85mm-enhanced-aluminum-4-6lambda-mirror/5355/)      | Smaller one should work as well, but this is a safe bet while not having any ghosting issues.                                                                                                                                                                                                                                                                                                                                                      |
| 3mm Elastic band (1m)          | 1,99€  | [AliExpress ("3mm 10meters" option)](https://www.aliexpress.us/item/1005008987955538.html)             | should be between 2mm and 3mm thickness, you won't actually need a meter, but cutting them to the right sizes sometimes takes a few tries.                                                                                                                                                                                                                                                                                                         |
| 26AWG Silicone wire            | 10.29€ | [AliExpress ("26 AWG 60m" option)](https://www.aliexpress.us/item/1005005450546335.html)               | Any 26AWG (or close to) silicone wire will work, would recommend least 3 unique colors though.                                                                                                                                                                                                                                                                                                                                                     |
| O-Ring OD32 CS3.1              | 2.49€  | [AliExpress 50pc ("OD32mm CS3.1mm" option)](https://www.aliexpress.us/item/1005008292700053.html)      | Kinda optional, it by itself won't water proof the housing, as there are other way water can still get in besides the edge of the camera-cage, but it's a start.                                                                                                                                                                                                                                                                                   |
| 7mm Momentary Button           | 2.99€  | [AliExpress 5pc ("PBS-110 NO Black") option](https://www.aliexpress.us/item/1005008777027854.html)     |                                                                                                                                                                                                                                                                                                                                                                                                                                                    |
| 1.25mm 5pin jst cable          | 2.69€  | [AliExpress ("5set 5pin" option)](https://www.aliexpress.us/item/1005008669994352.html)                |                                                                                                                                                                                                                                                                                                                                                                                                                                                    |
| Heat shrink tubes              | 1.69€  | [AliExpress](https://www.aliexpress.us/item/1005008858472748.html)                                     |                                                                                                                                                                                                                                                                                                                                                                                                                                                    |

# Build-Guide

## 3d printed parts

1. Print the `Body_Shell.step`, `PVS14_Lens_Clip.step`, and `Camera_Cage_x_x_deg.step` with a basic 0.12mm layer height profile.
Change the following
    1. Wall-loops to 3x or 4x. 
    2. Infill to about 22% to 35%
    3. Support to tree

    <br>
    <img src="Resources/3d_printed_parts_Pictures/Shell_Clip_and_CameraCage.jpg" alt="Shell_clip_and_cage_3d_printing" width="450"/>
    <br>
    I'd recommend adding extra support to the push-button mount hole on the housing shell
    <br>
    <img src="Resources/3d_printed_parts_Pictures/shell_button_extra_support.jpg" alt="Extra_support" width="300"/>

2. Print the `Display_Spacer.step`, and `Mirror_Cover_Thin.step` with <= 0.1mm layer height, and initial layer height to <= 0.2mm. I'd highly recommend printing these parts out of a dark, as matte as possible filament, to prevent light reflecting inside the periscope
<br>
<img src="Resources/3d_printed_parts_Pictures/cover_and_spacer.jpg" alt="Cover_and_Spacer_3d_print" width="600"/>

3. Print the periscope, pause it at the spot shown on the image (just after the slot for the mirror is done). I'd highly recommend printing this part out of a dark, as matte as possible filament, to prevent light reflecting inside the periscope 
<br>
<img src="Resources/3d_printed_parts_Pictures/stem_print_pause.jpg" alt="Print_Pause_Location" width="600"/>
<img src="Resources/3d_printed_parts_Pictures/Stem_Periscope.jpg" alt="Stem_Periscope_3d_print" width="200"/>
<br>
During the pause, insert the mirror, and mirror cover (with the hole in the cover facing to the top). After the print is done, press-fit the small 25mm DCX lens into place from the outside, for me this held very well, but I'll also add a part that can be glued in some time in the future.

4. (Optional, but recommended) print the `Bikini_Cover_TPU.step` out of a fairly soft TPU. I printed mine out of 95A Overture TPU


## Electronics

<img src="Resources/Schematic_BCOTI_mk1.png" alt="wiring_diagram" width="1000"/>

### Step-by-step

Using Heat shrink for all connections where possible is recommended.

1. Cut the cables of the USB-C port to a length of around 80mm.
2. Cut a 1x 80mm piece of (Red) wire. And 3x 30mm pieces of (Red) Wire. These will be your 5V-net. Strip a portion of the installation on one side so that you can twist together the 3x short pieces, then cover in solder.
3. Cut 5x 30mm pieces of black wire. These will be your ground-net. Strip one side and combine the 5pieces, same as you did for the 5v-net.
4. Solder the 80mm piece of (Red) wire, to the potentiometer's on-off-switch.
<br>
<img src="Resources/Electronics_Steps_Pictures/Poti_on_off_switch.jpg" alt="Poti_On_Off_Switch" width="450"/>
(picture shows USB-C Port wire already connector, this is done during assembly)

5. Solder the bundle of 3 (Red) wires to the 80mm piece from the previous step.
<br>
<img src="Resources/Electronics_Steps_Pictures/5v_net.jpg" alt="5v_net" width="350"/>

6. Attach the 5v wires, as well as ground to the USB-C breakout board, the display driver connector, and the camera connector, as shown in the diagram.

7. Connect ground wires to the push-button, and on-off-on toggle switch.

8. Cut a 50mm piece of wire, solder one end to the push button, and the other to under side, facing inwards to the esp32c3's GPIO21 pin.
<br>
<img src="Resources/Electronics_Steps_Pictures/push_button.jpg" alt="Push_button" width="450"/>
9. Cut the `brightness+` and `brightness-` cables of the display driver to a length of about 70mm, then attach them to the `on`-terminals of the toggle-switch.
<br>
<img src="Resources/Electronics_Steps_Pictures/display_brightness.jpg" alt="Brightness_switch" width="450"/>
10. Connect the white wire of the usb-c port to the D- Pad of the USB-C breakout board, and the blue wire to D+.
<br>
<img src="Resources/Electronics_Steps_Pictures/5v_usb.jpg" alt="USB_C_Port_to_connector" width="450"/>
11. Connect the RX wire of the camera to GPIO1, and the TX wire to GPIO2 of the esp32c3 board. The cables should be attached on the underside facing inwards.
<br>
<img src="Resources/Electronics_Steps_Pictures/UART_lines.jpg" alt="UART_lines" width="450"/>
12. Using 3x 50mm pieces of wire, connect the potentiometer to the esp32 board like shown in the schematic. Ground should be on the left, sense-wire (GPIO4) in the centre, and 3.3v on the right, that way the power goes from 0v to 3.3v when it's turned up, instead of being reversed.
<br>
<img src="Resources/Electronics_Steps_Pictures/poti.jpg" alt="Poti_Voltage_Dividier" width="450"/>
13. Connect the Video-out of the camera to the `CVBS+` cable of the display driver. Total wire length is recommended at 130mm. 

## Assembly

1. Insert the USB-C Port into the housing.

2. Solder the 5v wire of the Port to the on-off-switch of the potentiometer

3. Insert the potentiometer into the shell (from the inside) with the pins pointing down, and fix in place the with the Nut and washer.
<br>
<img src="Resources/Assembly_Steps/Poti_oriantation.png" alt="potentiometer oriantation" width="450"/>

4. Insert the Brightness Toggle switch (horizontal orientation), and fix in place with washer and nut.
<br>
<img src="Resources/Assembly_Steps/Toggle_Switch_Oriantation.jpg" alt="toggle switch oriantation" width="450"/>

5. With the USB-Port facing down, and the usb-c breakout connector inserted, insert the esp32 board into the centre slot of the shell
<br>
<img src="Resources/Assembly_Steps/Slots.jpg" alt="Shell Slots" width="450"/>

6. Disconnect the display from the driver, and insert the FPC through the hole of the slot for the display.

7. Connect the display to the driver again, then insert the driver (with the wires pointing into the shell) into the slot.

8. Insert the push-button into the slot on the bottom of the shell, and fix it in place with the nut and washer.

9. Wrap the O-Ring around the CameraCage, and insert the camera module (with the JST connector facing down, away from the slot in the top of the cage), then fix in place with all 6x M2x4 sews. Tighten them evenly, instead of doing one fully after the other.

10. Check if the camera cage can be fully inserted without putting pressure on the display FPC or any components inside the shell.

11. Insert the 11x heat inserts into the holes with your soldering iron.

12. Attach the Lens clip, Periscope, and Camera cage with the 11x M2x4 screws.

13. Attach the unit to your NV device and check the alignment of the image. (Should the image be mirrored incorrectly, adjust it via the WebUI's "Flip Mode" setting)
    1. Attach the unit like shown in the picture, so hanging straight down from your NV unit
    <br>
    <img src="Resources/Assembly_Steps/Allignment_check_oriantation.jpg" alt="bottom_hanging allignment" width="450"/>

    2. If the thermal image is too far left or right, check that the camera cage is properly installed. This usually isn't an issue.

    3. If the thermal image is too far up or down, you'll need to reprint the camera cage with a different offset. 1.2° down worked pretty well for me so far, but you can adjust it in the `Parameters` of the FreeCad file, then exporting the `CameraCage` again. If the image is too far up, then you need to reduce the angle, so that the camera looks up slightly more. If the image is too low, the angle needs to increase. There are a collection of different offsets in a `CAD_Files/Other_CameraCage_Offsets`
    <br>
    repeat this step until the unit is aligned well enough.

