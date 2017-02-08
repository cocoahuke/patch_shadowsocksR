#Make ShadowsocksX-R running in Macos 10.10

The following tutorial will help you use ShadowSocket in Macos10.10

I've been using older versions of ShadowSocket before, but then when my SS provider updated their server(New: Protocol/OBFS), older version of shadowsocks is no longer be use

Although the program can normally open, But one of executable program in application bundle (ShadowsocksX-R.app/Contents/Resources/ss-local),
This one is not running

The ss-local reference to a function called clock_gettime that's not exist in macos (Apple added in Sierra 10.12), so when executed to this function, Dyld occur error, ss-local exits. btw clock_gettime reference from libev

All we need have to do, its just replace this function, make it can work on macos

ShadowsocksX-NG won't supported, cause the CoreImage.framework framework is missing, This is difficult to solve except updating the system
```
dyld: Library not loaded: /System/Library/Frameworks/CoreImage.framework/Versions/A/CoreImage
  Referenced from: /Users/huke/Desktop/ShadowsocksX-NG.app/Contents/MacOS/ShadowsocksX-NG
  Reason: image not found
Trace/BPT trap: 5
```
## First step: **Download**

but that's okay, there is another ShadowsocksX-R which has pink icon  
New protocols and obfuscation protocols support !  
<img src="pink_icon.png" height="100"/>

download ShadowsocksX-R and unzip, Or if you already installed, then jump to Last step

release versions links are below
[https://github.com/yichengchen/ShadowsocksX-R/releases](https://github.com/yichengchen/ShadowsocksX-R/releases)

Latest release (1.3.9):
[https://github.com/yichengchen/ShadowsocksX-R/releases/tag/sst_1.3.9](https://github.com/yichengchen/ShadowsocksX-R/releases/tag/sst_1.3.9)

after we got app bundle, the application should be able to open normally. still can not be used though, cause lack of clock_gettime this function
## Second step: **Move to Applications folder**
As title, move the ShadowsocksX-R to the Applications folder

## Last step: **Run Script**

Just run this script
```bash
git clone https://github.com/cocoahuke/patch_shadowsocksR .git \
&& cd patch_shadowsocksR \
&& make \

```
