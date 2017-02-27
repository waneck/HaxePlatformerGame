# HaxePlatformerGame
This is a port of Unreal's `PlatformerGame` demo to use Unreal.hx. It is a 1:1 port, and you
can check the history to see how easy it is to port an existing C++ codebase to Haxe, and how C++ features map into
Haxe.

In order to test this example, please do the following:

```bash
git clone https://github.com/waneck/HaxePlatformerGame.git
cd HaxePlatformerGame
git submodule init
git submodule update # this is important, as the Unreal.hx plugin is defined as a git submodule
```
After the project is cloned and the submodules are initialized, you can either run:

```bash
cd PlatformerGame/Plugins/UnrealHx
haxe init-plugin.hxml
```
And start the Unreal editor, which will compile the needed files when you open them for the first time

You can alternatively do:
```bash
cd PlatformerGame
export UE4=/path/to/your/UnrealEngine/directory # if on windows, use SET instead of export
ant build
```
This requires ant to be installed, and will build your project entirely through the command-line


In order to compile and run this, you will need Haxe 3.4 and Unreal 4.14. If you're on Windows, Visual Studio 2015 is required.
