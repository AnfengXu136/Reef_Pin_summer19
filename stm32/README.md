#  STM32f103c8t6

There are different versions of our code. Each version implements a new feature. More information about each version can be found under each version.
Our document only concerns v0.2, v0.2.1, and v0.3.

## Information about this Microcontroller

### MCU
Since last summer, students have developed different versions of D-SEA using multiple MCUs. Last summer (2018), the REU students used MSP430 for low power consumption. 
During the following academic year, student volunteers on the project were using Arduino because of its libraries for new features and fast software development. Last quarter (SP19), three students were using STM32 for a wide range of peripherals, power, and reliability. Though we do not need much MCU calculating power, we have decided to continue working on the STM32 version because the power consumption is not an issue, and it is cheap (5$), reliable, and flexible for software development.
* the datasheet can be found under "datasheet" folder

### IDE and Library
Our embedded software is fully dependant on STM32 Standard Peripheral Libraries since we are using a STM32 microcontroller. The SW4STM32R IDE (System Workbench for STM32) is free and does not have a limit on the memory of the program. It is compatible with both Windows and Mac, and it is based on Eclipse. We use ST-Link to download software into the embedded chip. With ST-Link and SW4, we can use step-by-step debugging. For basic components, we use GPIO, USART, I2C, SPI, ADC, EXTI, PWR, and RTC in STM32 Std Peripheral libraries. The driver code for each module is developed separately. Most of the breakout sensors are provided with Arduino libraries that can instantly plug in. Basically, we are rewriting these libraries with STM32 backend. Most of the user logic is defined in main.c file. The embedded software is modularized and extendable for future teams. To install the SW4STM32R IDE, click on the following link:
http://www.openstm32.org/System%2BWorkbench%2Bfor%2BSTM32
You need to create an account for the download.

Setting up the environment - after downloading SW4STM32 and cloning our repo, choose the version you want to open and open our project by clicking the cproject file. 
![cproject file](https://github.com/AlexXu136/Reef_Pin_summer19/blob/master/stm32/cproject_file.png)

After opening the file, click “Run” and click again “Run Configuration.” This should open a window. 
![run configurations](https://github.com/AlexXu136/Reef_Pin_summer19/blob/master/stm32/run_configurations.png)

Then, go to “debugger” and change Reset Mode to “Software System Reset.”
![debugger](https://github.com/AlexXu136/Reef_Pin_summer19/blob/master/stm32/debugger_window.png)

Now, you should be able to code and upload your code.
To compile, click “build” and to upload the code, click “run.”
