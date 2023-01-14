# noteGenerator.py
# usage: python3 noteGenerator.py

from datetime import datetime

filename = "DAC_14_bit_notes.h"

with open(filename, 'w') as file:
    file.write("// DAC_14_bit_notes.h\n// This file was automatically generated by running \"python3 noteGenerator.py\" on " + datetime.now().strftime("%m/%d/%Y, %H:%M:%S" + "\n"))
    file.write("#ifndef _DAC_14_BIT_NOTES_H_\n")
    file.write("#define _DAC_14_BIT_NOTES_H_\n")
    file.write("""#include "Arduino.h" """ + "\n")
    file.write("const PROGMEM uint16_t NOTES2DAC[] = {")
    # DAC value = half step number * 1/12 (V/oct) * 2^14 (bits) / DAC range (V)
    dac_range = 5 # Volts
    max_note = dac_range * 12 # 5V * 12 notes per octave
    for note_idx in range(0, max_note + 1, 1):
        dac_value = note_idx * 1/12 * 2**14 / dac_range
        file.writelines(str(round(dac_value)))
        if (note_idx < max_note):
            file.writelines(", ")
    file.write("};\n")

    file.write("#endif // _DAC_14_BIT_NOTES_H_\n")

