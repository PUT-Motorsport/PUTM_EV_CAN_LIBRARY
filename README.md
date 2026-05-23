# PUTM_EV_CAN_LIBRARY

C++ wrapper library for STM32 HAL CAN communication in Formula Student Electric Vehicle projects.

## 📖 Overview

High-level abstraction layer over STM32 HAL CAN interface, simplifying CAN bus communication between ECUs in racing car electronics.

---

## 📂 Repository Structure
├── database/       # CAN message database and signal definitions  
├── documention/    # DOCUMENTATION - Read before use!  
├── include/        # Header files (.h, .hpp)  
├── src/            # Source files (.cpp)  
└── CMakeLists.txt  # Build configuration  

---

## 🚀 Quick Start

### ➕ Scenario 1: Adding the library for the first time (as a Git submodule)

1. Navigate to the `Core/Inc` folder in your project:
```bash
   cd <your_project>Core/Inc
```

2. Add the library as a submodule:
```bash
   git submodule add https://github.com/PUT-Motorsport/PUTM_EV_CAN_LIBRARY.git
```

3. Initialize and fetch the submodule contents:
```bash
   git submodule update --init --recursive
```

4. When cloning your project on another machine, always use:
```bash
   git clone --recurse-submodules <your_project_url>
```
   Or if already cloned without submodules:
```bash
   git submodule update --init --recursive
```

5. **Read the documentation in `documention/` before proceeding!**

---

### 🔄 Scenario 2: Library already added — updating to the latest version

1. Pull the latest changes from the library remote:
```bash
   git submodule update --remote PUTM_EV_CAN_LIBRARY
```

2. Stage and commit the submodule pointer update in your project:
```bash
   git add Core/Inc/PUTM_EV_CAN_LIBRARY
   git commit -m "chore: update PUTM_EV_CAN_LIBRARY to latest version"
```

3. If your teammates need to sync after you pushed:
```bash
   git submodule update --init --recursive
```

4. **Check the documentation for any breaking changes or HAL driver patching updates!**

---

## 📚 Documentation

**⚠️ READ THE DOCUMENTATION IN `documention/` BEFORE USING THIS LIBRARY!**

The documentation contains:
- API reference and usage examples
- Integration guide and configuration
- HAL driver patching instructions
- Troubleshooting and best practices

## 👥 Author & Contact

**Author:** Michał Błotniak

**Technical Contacts:**
- Michał Błotniak
- Tomasz Majchrzak  
- Filip Michalski

**Support:** [GitHub Issues](https://github.com/PUT-Motorsport/PUTM_EV_CAN_LIBRARY/issues)

---

## 🔗 Related Projects

- [PUTM_EV_CAN_DBC](https://github.com/PUT-Motorsport/PUTM_EV_CAN_DBC) - CAN database
- [PUTM_EV_TS_MASTER_CONFIGURATION](https://github.com/PUT-Motorsport/PUTM_EV_TS_MASTER_CONFIGURATION) - TsMaster template

---

<div align="center">

**PUTM_EV_CAN_LIBRARY** - Built by Michał Błotniak for PUT Motorsport

</div>
