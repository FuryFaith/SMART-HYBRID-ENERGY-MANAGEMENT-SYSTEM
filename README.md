## **SMART HYBRID ENERGY MANAGEMENT SYSTEM USING FUZZY LOGIC**
## Final year project by Aravindhakshan Seshadri

### Smart Hybrid Energy Management System
*The Smart Hybrid Energy Management System (SHEMS) is a critical home device for effective smart grid control at the demand side. This track and arranges different home appliances in real-time, using the human-machine interface to save power costs and maximize energy efficiency usage, based on consumer expectations in smart houses. With that concerns about global energy security and environmental pollution, increasingly dispersed renewable generations, such as wind turbines, solar panels, and plug-in electric vehicles (PEVs), etc., will be incorporated into the grid with slowly increasing penetration into active distribution grids.*

_Each home has two energy-related parts: consumption and generation. The energy usage component involves various home appliances and lamps. The energy generation component comprises renewable energies such as solar and wind energy. Because a home consumes and generates energy, to minimize the energy cost, a control device like a home server needs to monitor and control both energy consumption and energy generation._

_SHEMS monitors medium and light charges and determines whether the load should be connected to the battery or grid. Installed in the premises is a renewably energized battery bank. The battery bank provides medium- and light-load power. The grid is always connected to heavy loads. The power consumed by the loads is sensed, processed, and displayed on a webpage where the user can easily create control. SHEMS is the Master Controller and manages the activities of the whole Energy Management system.
The SHEMS system includes grid availability, connected load type (light or medium load), bank status of the battery, and daytime. Based on these inputs, it determines whether to smartly attach the charge to Grid or Battery. SHEMS collects information directly from loads and inspects it in Space. The State of Charge (SOC) of the two batteries are the inputs into the battery control system. SHEMS decides based on the inputs that the battery should be linked to the network and which battery should be charged to._


### Requirement
- Arduino IDE
- Proteus Design Suite (8.0 and above)

### Libraries Used
- [ACS712 By Murat Demirtas](https://github.com/muratdemirtas/ACS712-arduino-1)
- [Time by Paul Stoffregen](https://github.com/PaulStoffregen/Time)
- [DS1307RTC by PaulStoffregen](https://github.com/PaulStoffregen/DS1307RTC)
- [eFLL - A Fuzzy Library for Arduino and Embedded Systems](https://github.com/zerokol/eFLL) 
- Keypad Library - prebuilt in the IDE as a base.
- [Filter](https://github.com/hideakitai/Filters)

Link to `Demo Video` of my Real-Time Implementation. Click Here!

Link to `Research Paper`. [Click Here](http://ijsrem.com/download/smart-hybrid-energy-management-system-using-fuzzy-logic/?wpdmdl=3568&masterkey=5eeec2d6ef2ff)!

**Source Code and Schematic of the Simulation project is present in the library itself**
