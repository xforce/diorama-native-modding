# Diorama Native Modding
This is just a small side project of mine to add a C++ Api to the Windows Version of Diorama http://robtheswan.com http://www.twitch.tv/robtheswan

### Random Information
The loading and executing of diorama into my own executable is working
The next step is to get some basic hooking done and grab the PlayerSim object
and play around with it
After that I am going to implement plugin loading and all the fancy stuff

### Compile instruction
```
python configure.py
python build.py
```

### Execute instructions
You have to copy paste the diorama folder in to the Debug/Release folder and then run diorama_native_modding.exe

### Known Issues
Currently it crashes when the connect to the server is finished
Single player works though