# MyMiniEDR
My Mini EDR driver to learn about kernel callbacks and EDR detecions. It's work in progress, just releasing it to share code for a short workshop atm :)

# Building
- [Visual Studio 2022](https://visualstudio.microsoft.com/vs/)
- Windows Driver Kit (WDK) for Windows 10/11
- Enable Kernel Mode Driver (KMDF) development features during install
- A test-signing enabled VM or test system (for unsigned drivers)
- Inside the project properties go to Linker -> Command Line, and add the option `/integritycheck` to be able to disable the integrity check. This makes the registration of kernel callbacks possible. Prevents NTSTATUS `0xC0000022`  error on callback registration.

# Kernel Callbacks
- [x] Process Creation Kernel Callbacks (`PsSetCreateProcessNotifyRoutine`, `PsSetCreateProcessNotifyRoutineEx`, `PsSetCreateProcessNotifyRoutineEx2`)
- [ ] Thread Creation Kernel Callbacks (`PsSetCreateThreadNotifyRoutine`)
- [ ] Image Load Kernel Callbacks (`PsSetLoadImageNotifyRoutine`)
- [ ] Registry Callbacks (`CmRegisterCallbackEx`)
- [ ] Object Callbacks (`ObRegisterCallbacks`)
  - [ ] Process handle operations (OB_TYPE_PROCESS)
  - [ ] Thread handle operations (OB_TYPE_THREAD)
- [ ] File System Filter Callbacks (MiniFilter driver: `FltRegisterFilter`)
- [ ] Network Filter Callbacks (WFP Callouts / NDIS filter drivers)

# Credits
- The CETP course from [Altered Security](https://www.alteredsecurity.com/evasionlab)
- The Evading EDR book from [Matt Hand](https://www.amazon.nl/-/en/Matt-Hand/dp/1718503342)
