Collection of custom scripts, utils and weapons i have introduced in Worms Armageddon Project X (WA:PX) a WormKit module written in Pascal that, for some reason, uses a C-Like language called EAX/PXC.

## Requirements

- Worms Armageddon version **3.6.31.0** with WormKit installed.
- WormKit DLLs that allow the game's interface to show up:
- wkFrontendFix.DLL
- wkAntiLag.DLL
- wkWndMode.DLL (Stopped most crashes after finishing games)
- Optional ddraw.DLL that also makes the whole thing more stable.
**All of these DLLs can be found on 0rang3's PX build.**

## Usage

- This is a repo that is meant to just show what i've done over the past months. Regardless of that, i will still try to include PXL updates, sometimes more often than the code itself.
- Move all PXL files inside Worms\Libs
- Enjoy =)

## Misc

- If you want to see interesting stuff, check utils_red, or the saw/bees, these are my favorite! flowers and random teleport are quite cool too
- Attack helicopter is also nice


#Contributions to other libs:

#SPB:

- Added support for sprite multiplexing (sprite sheets).
- Added support for weapons having firing sprites.
- Added support for weapons to change their sprite at any given moment.

#Others:

- Particles.PXL Support for sprite sheets and animations.
- Fixed Armor Vest (mostly).
- Helped orange a bunch.
- Fixed icy arrow freezing random stuff.
- Added flag for magnets to ignore objects (obj->isMagnetic = false)
- Fixed Portal Gun's 8 simultaneous FOR loops lol
- Added useful stuff to utils.pxl
- Fixed stone's erratic behavior regarding magnets and antiglitch
- And most important of all, added a bunch of methods that will make modding a LOT easier!
- Added bees



