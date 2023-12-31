/*sSense_BME680.h

version adapted by itbrainpower.net for:
	s-Sense BME680 I2C sensor breakout - temperature, humidity, pressure and gas - [PN: SS-BME680#I2C, SKU: ITBP-6003], info https://itbrainpower.net/sensors/BME680-BVOC-TEMPERATURE-HUMIDITY-PRESSURE-I2C-sensor-breakout 

based on Zanshin_BME680 library version 1.0.2 / 2019-01-26 - https://github.com/SV-Zanshin
Amazing work Zanshin!
*/

/*! @file Zanshin_BME680.h

@mainpage Arduino Library to control a Bosch BME Sensor

@section Zanshin_BME680_section Description

Class definition header for the Bosch BME680 temperature / humidity / pressure sensor. The sensor is described at 
https://www.bosch-sensortec.com/bst/products/all_products/BME680 and the datasheet is available from Bosch at 
https://ae-bst.resource.bosch.com/media/_tech/media/datasheets/BST-BME680-DS001-00.pdf \n\n

The BME680 can use either SPI or I2C for communications. This library allow I2C at various bus speeds.\n\n

The original version of the library is available at https://github.com/SV-Zanshin/BME680 and extensive 
documentation of the library as well as example programs are described in the project's wiki pages located at
https://github.com/SV-Zanshin/BME680/wiki. \n\n

@section license License

This program is free software: you can redistribute it and/or modify it under the terms of the GNU General
Public License as published by the Free Software Foundation, either version 3 of the License, or (at your
option) any later version. This program is distributed in the hope that it will be useful, but WITHOUT ANY
WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details. You should have received a copy of the GNU General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>.

@section author Author

Written by Arnd\@SV-Zanshin

@section versions Changelog

Version | Date       | Developer                     | Comments
------- | ---------- | ----------------------------- | --------
1.0.2   | 2019-01-26 | https://github.com/SV-Zanshin | Issue #3 - Converted documentation to doxygen style
1.0.1   | 2018-07-22 | https://github.com/SV-Zanshin | Corrected I2C datatypes
1.0.1   | 2018-07-03 | https://github.com/SV-Zanshin | Issue #1. Added waitForReading and parameter to getSensorData()
1.0.0   | 2018-07-02 | https://github.com/SV-Zanshin | Added guard code against multiple I2C constants definitions
1.0.0   | 2018-07-01 | https://github.com/SV-Zanshin | Added and tested I2C, SPI and software SPI connections
1.0.0a  | 2018-06-30 | https://github.com/SV-Zanshin | Cloned from BME280 library and started recoding
*/
#include "Arduino.h" // Arduino data type definitions
#include <Wire.h>    // Standard I2C "Wire" library
//#include <SPI.h>     // Standard SPI library
#ifndef BME680_h

	#ifdef  __SAMD21G18A__
	  #define DebugPort SerialUSB
	#else
	  #define DebugPort Serial 
	#endif

  /** @brief  Guard code definition for the library header*/
  #define BME680_h
  #define CONCAT_BYTES(msb, lsb) (((uint16_t)msb << 8) | (uint16_t)lsb) ///< Inline to combine msb and lsb
  /*****************************************************************************************************************
  ** Declare constants used in the class                                                                          **
  *****************************************************************************************************************/
  #ifndef I2C_MODES
  /** @brief  Guard code definition for the I2C modes */
  #define I2C_MODES
    const uint32_t I2C_STANDARD_MODE              =  100000; ///< Default normal I2C 100KHz speed
    const uint32_t I2C_FAST_MODE                  =  400000; ///< Fast mode
    const uint32_t I2C_FAST_MODE_PLUS             = 1000000; ///< Really fast mode
    const uint32_t I2C_HIGH_SPEED_MODE            = 3400000; ///< Turbo mode
  #endif
  /*
  const uint32_t SPI_HERTZ                        =  500000; ///< SPI speed in Hz
  */
  const uint8_t  BME680_STATUS_REGISTER           =    0x1D; ///< Device status register
  const uint8_t  BME680_GAS_HEATER_REGISTER0      =    0x5A; ///< Heater Register 0 address
  const uint8_t  BME680_GAS_DURATION_REGISTER0    =    0x64; ///< Heater Register 0 address
  const uint8_t  BME680_CONTROL_GAS_REGISTER1     =    0x70; ///< Gas control register on/off
  const uint8_t  BME680_CONTROL_GAS_REGISTER2     =    0x71; ///< Gas control register settings
  const uint8_t  BME680_CONTROL_HUMIDITY_REGISTER =    0x72; ///< Humidity control register
  const uint8_t  BME680_SPI_REGISTER              =    0x73; ///< Status register for SPI memory
  const uint8_t  BME680_CONTROL_MEASURE_REGISTER  =    0x74; ///< Temp, Pressure control register
  const uint8_t  BME680_CONFIG_REGISTER           =    0x75; ///< Configuration register
  const uint8_t  BME680_CHIPID_REGISTER           =    0xD0; ///< Chip-Id register
  const uint8_t  BME680_SOFTRESET_REGISTER        =    0xE0; ///< Reset when 0xB6 is written here
  const uint8_t  BME680_CHIPID                    =    0x61; ///< Hard-coded value 0x61 for BME680
  const uint8_t  BME680_RESET_CODE                =    0xB6; ///< Reset when this put in reset reg
  /*****************************************************************************************************************
  ** Declare enumerated types used in the class                                                                   **
  *****************************************************************************************************************/
  /*! @brief  Enumerate the sensor type */
  enum sensorTypes       {TemperatureSensor,HumiditySensor,PressureSensor,GasSensor,UnknownSensor};
  /*! @brief  Enumerate the Oversampling types */
  enum oversamplingTypes {SensorOff,Oversample1,Oversample2,Oversample4,Oversample8,Oversample16,UnknownOversample };
  /*! @brief  Enumerate the iir filter types */
  enum iirFilterTypes    {IIROff,IIR2,IIR4,IIR8,IIR16,IIR32,IIR64,IIR128,UnknownIIR };
  
  /*!
* @class BME680_Class
* @brief Main BME680 class for the temperature / humidity / pressure sensor
*/
  class BME680_Class 
  {
    public:
      BME680_Class();
      ~BME680_Class();
      bool     begin();                                                       // Start using I2C Communications   //
      bool     begin(const uint32_t i2cSpeed);                                // I2C with a non-default speed     //
      /*
	  bool     begin(const uint8_t chipSelect);                               // Start using hardware SPI         //
      bool     begin(const uint8_t chipSelect, const uint8_t mosi,            // Start using software SPI         //
                     const uint8_t miso, const uint8_t sck);                  //                                  //
      */
	  bool     setOversampling(const uint8_t sensor, const uint8_t sampling); // Set enum sensorType Oversampling //
      bool     setGas(uint16_t GasTemp, uint16_t GasMillis);                  // Gas heating temperature and time //
      uint8_t  setIIRFilter(const uint8_t iirFilterSetting=UINT8_MAX);        // Set IIR Filter and return value  //
      void     getSensorData(int32_t &temp, int32_t &hum,                     // get most recent readings         //
                             int32_t &press, int32_t &gas,                    //                                  //
                             const bool waitSwitch = true);                   //                                  //
      void     reset();                                                       // Reset the BME680                 //
    private:
      bool     commonInitialization();                       ///< Common initialization code
      uint8_t  readByte(const uint8_t addr);                 ///< Read byte from register address
      void     readSensors(const bool waitSwitch);           ///< read the registers in one burst
      void     waitForReadings();                            ///< Wait for readings to finish
      void     getCalibration();                             ///< Load calibration from registers
      uint8_t  _I2CAddress         = 0;                      ///< Default is no I2C address known
      /*
	  uint8_t  _cs,_sck,_mosi,_miso;                         ///< Hardware and software SPI pins
      */
	  uint8_t  _H6,_P10,_res_heat_range;                     ///< unsigned configuration variables
      int8_t   _H3,_H4,_H5,_H7,_G1,_G3,_T3,_P3,_P6,_P7,_res_heat_val,_range_sw_error; ///< signed configuration variables
      uint16_t _H1,_H2,_T1,_P1;                              ///< unsigned 16bit configuration variables
      int16_t  _G2,_T2,_P2,_P4,_P5,_P8,_P9;                  ///< signed 16bit configuration variables
      int32_t  _tfine,_Temperature,_Pressure,_Humidity,_Gas; ///< signed 32bit configuratio variables

      /*************************************************************************************************************
      ** Declare the getData and putData methods as template functions. All device I/O is done through these two  **
      ** functions regardless of whether I2C, hardware SPI or software SPI is being used. The two functions are   **
      ** designed so that only the address and a variable are passed in and the functions determine the size of   **
      ** the parameter variable and reads or writes that many bytes. So if a read is called using a character     **
      ** array[10] then 10 bytes are read, if called with a int8 then only one byte is read. The return value, if **
      ** used, is the number of bytes read or written                                                             **
      ** This is done by using template function definitions which need to be defined in this header file rather  **
      ** than in the c++ program library file.                                                                    **
      *************************************************************************************************************/
      /*!
          @brief     Template for reading from I2C or SPI using any data type
          @details   As a template it can support compile-time data type definitions
          @param[in] addr Memory address
          @param[in] value Data Type "T" to read
          @return    Size of data read in bytes
      */
      template< typename T > uint8_t &getData(const uint8_t addr,T &value)
      {
        uint8_t* bytePtr    = (uint8_t*)&value;                              // Pointer to structure beginning
        static uint8_t  structSize = sizeof(T);                              // Number of bytes in structure
        if (_I2CAddress) {                                                   // Using I2C if address is non-zero
          Wire.beginTransmission(_I2CAddress);                               // Address the I2C device
          Wire.write(addr);                                                  // Send register address to read
          Wire.endTransmission();                                            // Close transmission
          Wire.requestFrom(_I2CAddress, sizeof(T));                          // Request 1 byte of data
          structSize = Wire.available();                                     // Use the actual number of bytes
          for (uint8_t i=0;i<structSize;i++) *bytePtr++ = Wire.read();       // loop for each byte to be read
        } 
		/*
		else {
          if (_sck==0) {                                                     // if sck is zero then hardware SPI
            SPI.beginTransaction(SPISettings(SPI_HERTZ,MSBFIRST,SPI_MODE0)); // Start the SPI transaction
            digitalWrite(_cs, LOW);                                          // Tell BME680 to listen up
            SPI.transfer(addr | 0x80);                                       // bit 7 is high, so read a byte
            for (uint8_t i=0;i<structSize;i++) *bytePtr++ = SPI.transfer(0); // loop for each byte to be read
            digitalWrite(_cs, HIGH);                                         // Tell BME680 to stop listening
            SPI.endTransaction();                                            // End the transaction
          } else {                                                           // otherwise we are using soft SPI
            int8_t i,j;                                                      // Loop variables
            uint8_t reply;                                                   // return byte for soft SPI transfer
            digitalWrite(_cs, LOW);                                          // Tell BME680 to listen up
            for (j=7; j>=0; j--) {                                           // First send the address byte
              digitalWrite(_sck, LOW);                                       // set the clock signal
              digitalWrite(_mosi, ((addr)|0x80)&(1<<j));                     // set the MOSI pin state
              digitalWrite(_sck, HIGH);                                      // reset the clock signal
            } // of for-next each bit
            for (i=0;i<structSize;i++) {                                     // Loop for each byte to read
              reply = 0;                                                     // reset our return byte
              for (j=7; j>=0; j--) {                                         // Now read the data at that byte
                reply <<= 1;                                                 // shift buffer one bit left
                digitalWrite(_sck, LOW);                                     // set and reset the clock signal
                digitalWrite(_sck, HIGH);                                    // pin to get the next MISO bit
                if (digitalRead(_miso)) reply |= 1;                          // read the MISO bit, add to reply
              } // of for-next each bit
              *bytePtr++ = reply;                                            // Add byte just read to return data
            } // of for-next each byte to be read
            digitalWrite(_cs, HIGH);                                         // Tell BME680 to stop listening
          } // of  if-then-else we are using hardware SPI
        } // of if-then-else we are using I2C
		*/
        return(structSize);
      } // of method getData()

      /*!
          @brief     Template for writing to I2C or SPI using any data type
          @details   As a template it can support compile-time data type definitions
          @param[in] addr Memory address
          @param[in] value Data Type "T" to write
          @return    Size of data written in bytes
      */
      template<typename T>uint8_t &putData(const uint8_t addr,const T &value)
      {
        const uint8_t* bytePtr = (const uint8_t*)&value;                     // Pointer to structure beginning
        static uint8_t  structSize = sizeof(T);                              // Number of bytes in structure
        if (_I2CAddress) {                                                   // Using I2C if address is non-zero
          Wire.beginTransmission(_I2CAddress);                               // Address the I2C device
          Wire.write(addr);                                                  // Send register address to write
          for (uint8_t i=0;i<sizeof(T);i++) Wire.write(*bytePtr++);          // loop for each byte to be written
          Wire.endTransmission();                                            // Close transmission
        }
		/*else {
          if (_sck==0) {                                                     // if sck is zero then hardware SPI
            SPI.beginTransaction(SPISettings(SPI_HERTZ,MSBFIRST,SPI_MODE0)); // start the SPI transaction
            digitalWrite(_cs, LOW);                                          // Tell BME680 to listen up
            SPI.transfer(addr & ~0x80);                                      // bit 7 is low, so write a byte
            for (uint8_t i=0;i<structSize;i++) SPI.transfer(*bytePtr++);     // loop for each byte to be written
            digitalWrite(_cs, HIGH);                                         // Tell BME680 to stop listening
            SPI.endTransaction();                                            // End the transaction
          } else {                                                           // Otherwise soft SPI is used
            int8_t i,j;                                                      // Loop variables
            uint8_t reply;                                                   // return byte for soft SPI transfer
            for (i=0;i<structSize;i++) {                                     // Loop for each byte to read
              reply = 0;                                                     // reset our return byte
              digitalWrite(_cs, LOW);                                        // Tell BME680 to listen up
              for (j=7; j>=0; j--) {                                         // First send the address byte
                digitalWrite(_sck, LOW);                                     // set the clock signal
                digitalWrite(_mosi, (addr&~0x80)&(1<<j));                    // set the MOSI pin state
                digitalWrite(_sck, HIGH);                                    // reset the clock signal
              } // of for-next each bit
              for (j=7; j>=0; j--) {                                         // Now read the data at that byte
                reply <<= 1;                                                 // shift buffer one bit left
                digitalWrite(_sck, LOW);                                     // set the clock signal
                digitalWrite(_mosi, *bytePtr&(1<<j));                        // set the MOSI pin state
                digitalWrite(_sck, HIGH);                                    // reset the clock signal
              } // of for-next each bit
              digitalWrite(_cs, HIGH);                                       // Tell BME680 to stop listening
              uint8_t dummy = *bytePtr++;                                                    // go to next byte to write
            } // of for-next each byte to be read
          } // of  if-then-else we are using hardware SPI
        } // of if-then-else we are using I2C
        */
		return(structSize);
      } // of method putData()
  }; // of BME680 class definition
#endif