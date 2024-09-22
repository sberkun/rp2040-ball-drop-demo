
# Bill of Materials

## Physical Components 

| Item     |  Link (may or may not work) | Description |
| -------- | ------- | --- |
| Stepper Motor  | [amazon](https://www.amazon.com/gp/product/B00PNEQKC0)  | For spinning the disk. This NEMA 17 motor worked well for me.  |
| Flange | [amazon](https://www.amazon.com/gp/product/B08334MFVT) | For attaching the disk to the shaft of the motor. My motor has a 5mm shaft, so a flange with a 5mm inner diameter is needed. |
| Acrylic Tube | [amazon](https://www.amazon.com/gp/product/B0B9C46QJG) | Drop tube for the ball. My tube has a 12mm inner diameter, 15mm outer diameter, and a length of 14 inches (356mm). A longer tube may make the demo perform better.
| Electromagnet | [amazon](https://www.amazon.com/gp/product/B01N3387AA) | An electromagnet for holding and dropping the ball. This 25N magnet was more than strong enough to hold the ball to its face, but not strong enough to hold it through the acrylic tube wall. A larger electromagnet might be able to hold through the acrylic, allowing the drop to start slightly inside the tube and making the drop more consistent. |
| Photogate | [digikey](https://www.digikey.com/en/products/detail/liteon/LTH-301-32/3198349)   | For detecting the position of the ball in the drop tube. It's important to match the gate width to the outer diameter of the drop tube. This LTH-301-32 has a width of 15mm.  |
| Button | [adafruit](https://www.adafruit.com/product/1192) | A 60mm arcade button. This is overkill but fun to use. | 
| Bearing Ball | [amazon](https://www.amazon.com/gp/product/B09C1J2JPS) | One 11mm bearing ball. You can buy a pack of 11mm balls, or an assortment of different sizes. |
|Miscellaneous | | You will probably want hot glue, zip ties, screws (especially M3 screws for the flange), and plywood or plastic to build the frame. I used standard PLA for the frame and 1/8th inch plywood for the disk. |


## Electrical Components

| Item     |  Link (may or may not work) | Description |
| -------- | ------- | --- |
Microcontroller | [adafruit](https://www.adafruit.com/pico) | This demo nominally uses the Raspberry Pi Pico (an RP2040-based board), but you may be interested in testing a microcontroller of your choice. |
| Power Supply | [amazon](https://www.amazon.com/JOVNO-100-240V-Converter-Transformer-5-5x2-5mm/dp/B0875WMYCX) | Higher voltage means faster motor speeds, but also higher risk of blowing stuff up (particularly the electromagnet, which is supposed to be 5v). I like 12v but the sky is the limit. |
| Stepper motor driver | [amazon](https://www.amazon.com/gp/product/B07BND65C8) | Match to your stepper motor. NEMA 17 steppers can use A4988 or knockoff drivers. |
| MOSFET module| [amazon](https://www.amazon.com/gp/product/B07NWD8W26) | For driving the electromagnet. Anything goes here (MOSFET module, DC motor driver, or wiring up a MOSFET and diode yourself). |
| Miscellaneous | | You will probably want a breadboard, large capacitor (unless you really trust the cheap power supply you bought), jumper wires, resistors, and LEDs. |
