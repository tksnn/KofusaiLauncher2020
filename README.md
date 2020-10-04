# KofusaiLauncher2020
This is Launcher Program for 2020 Online Kofusai in Waseda High School.
## Caution
- This Program use TCP Networking by Port 51126 to [Server Program](https://github.com/tksnn/KofusaiTimer2020).<br>
- If you do not want to use network communication, set it from Config.
## Library
- C++17 ver.VS2020<br>
- Win32 API
- [OpenSiv3D ver.0.4.3](https://github.com/Siv3D/OpenSiv3D)<br>
- [HTTPClient with curl on OpenSiv3D](https://github.com/Siv3D/OpenSiv3D/issues/482)<br>
## Config
"launcher.ini" is Config File
```
[Config]
network = true <-Whether to use network communication
asketime = true <-Whether to ask for the PC number first
pcno = 12 <-Specifying the PC number
bgm = true <-Whether to play BGM
fps = true <-Whether to display FPS
logo = true <-Whether to display Logo Scene first
```
## Author
S.Takahashi (Waseda High School 125th)
## License
**Apache License 2.0**<br>
*A permissive license whose main conditions require preservation of copyright and license notices. Contributors provide an express grant of patent rights. Licensed works, modifications, and larger works may be distributed under different terms and without source code.*

Copyright (c) 2020 Waseda High School Personal Computing Club All Rights Reserved.<br>
The copyright of each game exists for each creator.(Games Folder)
