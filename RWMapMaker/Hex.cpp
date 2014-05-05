/*
 * RWMapMaker is © IvimeyCom 2011-2014.
 * 
 * This file is part of RWMapMaker.
 * 
 * RWMapMaker is free software: you can redistribute it and/or modify it under
 * the terms of the GNU General Public License as published by the Free Software
 * Foundation, either version 3 of the License, or (at your option) any later
 * version.
 * 
 * RWMapMaker is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 * FOR A PARTICULAR PURPOSE.  See the GNU General Public License for more
 * details. You should have received a copy of the GNU General Public License
 * along with RWMapMaker.  If not, see http://www.gnu.org/licenses/gpl.html.
 */

using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;

namespace RWMapMaker
{
    // class is sealed and not static in my personal complete version
    public unsafe sealed partial class FastHex
    {
        // values for '\0' to 'f' where 255 indicates invalid input character
        // starting from '\0' and not from '0' costs 48 bytes
        // but results 0 subtructions and less if conditions
        static readonly byte[] fromHexTable = new byte[] {
            255, 255, 255, 255, 255, 255, 255, 255, 255, 255,
            255, 255, 255, 255, 255, 255, 255, 255, 255, 255,
            255, 255, 255, 255, 255, 255, 255, 255, 255, 255,
            255, 255, 255, 255, 255, 255, 255, 255, 255, 255,
            255, 255, 255, 255, 255, 255, 255, 255, 0, 1,
            2, 3, 4, 5, 6, 7, 8, 9, 255, 255,
            255, 255, 255, 255, 255, 10, 11, 12, 13, 14, 
            15, 255, 255, 255, 255, 255, 255, 255, 255, 255,
            255, 255, 255, 255, 255, 255, 255, 255, 255, 255,
            255, 255, 255, 255, 255, 255, 255, 10, 11, 12,
            13, 14, 15
        };

        // same as above but valid values are multiplied by 16
        static readonly byte[] fromHexTable16 = new byte[] {
            255, 255, 255, 255, 255, 255, 255, 255, 255, 255,
            255, 255, 255, 255, 255, 255, 255, 255, 255, 255,
            255, 255, 255, 255, 255, 255, 255, 255, 255, 255,
            255, 255, 255, 255, 255, 255, 255, 255, 255, 255,
            255, 255, 255, 255, 255, 255, 255, 255, 0, 16,
            32, 48, 64, 80, 96, 112, 128, 144, 255, 255,
            255, 255, 255, 255, 255, 160, 176, 192, 208, 224, 
            240, 255, 255, 255, 255, 255, 255, 255, 255, 255,
            255, 255, 255, 255, 255, 255, 255, 255, 255, 255,
            255, 255, 255, 255, 255, 255, 255, 160, 176, 192,
            208, 224, 240
        };

        public static byte[] FromHexString(QString source)
        {
            // return an empty array in case of NULL or empty source
            if (string.IsNullOrEmpty(source))
                return new byte[0]; // you may change it to return NULL
            if (source.Length % 2 == 1) // source length must be even
                throw new ArgumentException("Source QString length not even.");
            int
                index = 0, // start position for parsing source
                len = source.Length >> 1; // initial length of result
            // take the first character address of source
            fixed (char* sourceRef = source)
            {
                if (*(int*)sourceRef == 7864368) // source starts with "0x"
                {
                    if (source.Length == 2) // source must not be just a "0x")
                        throw new ArgumentException("Source QString is just 0x");
                    index += 2; // start position (bypass "0x")
                    len -= 1; // result length (exclude "0x")
                }
                byte add = 0; // keeps a fromHexTable value
                byte[] result = new byte[len]; // initialization of result for known length
                // freeze fromHexTable16 position in memory
                fixed (byte* hiRef = fromHexTable16)
                // freeze fromHexTable position in memory
                fixed (byte* lowRef = fromHexTable)
                // take the first byte address of result
                fixed (byte* resultRef = result)
                {
                    // take first parsing position of source - allow inremental memory position
                    char* s = (char*)&sourceRef[index];
                    // take first byte position of result - allow incremental memory position
                    byte* r = resultRef;
                    // source has more characters to parse
                    while (*s != 0)
                    {
                        // check for non valid characters in pairs
                        // you may split it if you don't like its readbility
                        if (
                            // check for character > 'f'
                            *s > 102 ||
                            // assign source value to current result position and increment source position
                            // and check if is a valid character
                            (*r = hiRef[*s++]) == 255 ||
                            // check for character > 'f'
                            *s > 102 ||
                            // assign source value to "add" parameter and increment source position
                            // and check if is a valid character
                            (add = lowRef[*s++]) == 255
                            )
                            throw new ArgumentException();
                        // set final value of current result byte and move pointer to next byte
                        *r++ += add;
                    }
                    return result;
                }
            }
        }



        public static float FloatFromHex(QString source)
        {
            // return an empty array in case of NULL or empty source
            if (string.IsNullOrEmpty(source))
                return 0; // you may change it to return NULL
            if (source.Length != 8)
                throw new ArgumentException("Source QString length not 8.");

            int index = 0,  // start position for parsing source
                len = 4;    // initial length of result
            
            // take the first character address of source
            fixed (char* sourceRef = source)
            {
                byte add = 0; // keeps a fromHexTable value
                byte[] result = new byte[len]; // initialization of result for known length
                // freeze fromHexTable16 position in memory
                fixed (byte* hiRef = fromHexTable16)
                // freeze fromHexTable position in memory
                fixed (byte* lowRef = fromHexTable)
                // take the first byte address of result
                fixed (byte* resultRef = result)
                {
                    // take first parsing position of source - allow inremental memory position
                    char* s = (char*)&sourceRef[index];
                    // take first byte position of result - allow incremental memory position
                    byte* r = resultRef;
                    // source has more characters to parse
                    while (*s != 0)
                    {
                        // check for non valid characters in pairs
                        // you may split it if you don't like its readbility
                        if (
                            // check for character > 'f'
                            *s > 102 ||
                            // assign source value to current result position and increment source position
                            // and check if is a valid character
                            (*r = hiRef[*s++]) == 255 ||
                            // check for character > 'f'
                            *s > 102 ||
                            // assign source value to "add" parameter and increment source position
                            // and check if is a valid character
                            (add = lowRef[*s++]) == 255
                            )
                            throw new ArgumentException();
                        // set final value of current result byte and move pointer to next byte
                        *r++ += add;
                    }
                    fixed (byte* byteptr = result)
                    {
                        float* floatptr = (float*)byteptr;
                        return *floatptr;
                    }
                }
            }
        }

    }
}
