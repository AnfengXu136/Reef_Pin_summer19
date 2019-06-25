# Smart Integrated Tile
## Porject Overview
### Abstract
In our project we explore different hardware choices in order to develop a cheap, reliable depth sensor for Scripps Institute of Oceanography. The sensor will log and display depth readings in relatively shallow, high-illumination conditions in coral reef communities. The depth information will then be used in conjunction with photographs to create 3D models of the coral reefs. The current method for collecting this depth information is by manually measuring the depth just above the coral reefs, which is strenuous for the divers and prone to errors. We hope to provide a reliable alternative that automates this process and increases the precision of the measurements. Major design decisions include choosing a method of displaying depth (and later heading) data and how to quickly and easily transfer the collected information while maintaining the integrity of the waterproofing at an affordable cost.

### 100 Island Challenge
Our project falls within the greater context of the 100 Island Challenge, headed by the Scripps Institute of Oceanography. The 100 Island Challenge is a project that is collecting photographic, geographic, and oceanographic data in order to build models of coral reef communities around the world, and track their changes in structure and growth over time. More information on the challenge can be found [here](http://100islandchallenge.org/).

### Requirements
Researchers from Scripps Institution of Oceanography wish to work with this device under the following conditions: Bring the device on a boat and turn it on before going diving with the corals. Then they dive down and place the devices on the coral reef, and the device starts collecting depth information and displaying it on the screen. The divers swim back and forth to do the photomosaic survey, capturing images of the devices and the data shown on the screen. After that, they collect the devices and return to the boat. They read the data from the devices and download it for backup. Finally, they turn off the device and charge the device overnight.

### Objectives
Based on the requirements from Scripps Institution of Oceanography, we are building a device that can measure and display depth information when it’s placed on coral reefs. Then the depth information could be captured by cameras and retrieved from images. The other detailed functions are described as follows.
* Switch: A switch is needed to make sure the device can turn on before diving and turn off when it’s back to land. 
* Waterproof: Since the device needs to be working under 10-15 meters of salt water for around 4 hours, it should be strictly waterproof to avoid erosion. Waterproof also means that there may not be any buttons or plugs on this device.
* Clear display: The device works in relatively shallow, high-illumination conditions in coral reef communities. There will be sunlight, wind, and wave changing over time. The display module should display the depth information as clear as possible under different conditions to make sure the information can be retrieved from the images accurately.
* Rechargeable: The device needs to be reusable and rechargeable. Since the device is strictly waterproof, wireless charging is a better option compared to using plug and cable. The battery needs to be working for around 4 hours and can be charged fully over one night. In addition, there should be a strategy such as LED to indicate how much battery left.
* Data storage: There may be cases that researchers fail to retrieve accurate depth information from unclear images. For backup consideration, this device should be integrated with a data storage module such as a SD-card.
* Data Transmission: In order to download the depth data, a reliable wireless data transmission module is needed.

### Previous Work
A GitHub repository containing work done by a previous team on this project can be found [here](https://github.com/UCSD-E4E/coral-reef-mapping). Their work and documentation has provided us with valuable guidance for this project.

## Repo Organization
The stm32 folder contains the code needed to make our STM32F103C8T6 MCU work with the other components we have selected. The pcb folder includes pcb hardware design files for our device. The GUI folder includes files about data transmission and user interface.

## Team Members
* Liren "Tony" Chen
* Xuanyi "Ivy" Yu
* Samuel Givens

## Resources
* [Project Overview](https://drive.google.com/open?id=1Pus6sJ2qkKndr-W4F6nYFhtnWSXx7cwhct0EHbMCYVM)
* [Project Specification](https://drive.google.com/open?id=1e_64vk2hLm1WlwG5i0ryWBhBv_sy5NsdDfYOwhg4feA)
* [Oral Project Update](https://drive.google.com/open?id=1aEZSikz7JAXXPAU3vB3pFT56Vzhgw5z4WsIUU1GbcwE)
* [Milestone Report](https://drive.google.com/open?id=1EIhdwy2Vkavuux3ikYEyF9vRjz64th1LstWmFr8qIqA)
* [Final Oral Presentation](https://drive.google.com/open?id=1kZK5IM6NcCPRbsSYnko_ywsIkHWHUEnm4WjG7Nfi0os)
* [Final Project Video]()
* [Final Report](https://drive.google.com/open?id=1XefbvaYMcaKPTqKhkapIGFox0DjAbMtDtrPGPohWkk4)
