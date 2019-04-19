![Logo](https://cdn.xaymar.com/obs-stream-effects/logo.svg)
Bring your stream to life with more modern effects! Stream Effects adds several much needed features to OBS Studio, such as real time Blur and 3D Transform. Now you can blur out sources you think may be questionable, add sick 3D effects or recreate the Heroes of the Storm displacement overlay with the Displacement filter. The choice is yours, the possibilities are endless.

#### Status: [Windows: ![AppVeyor](https://ci.appveyor.com/api/projects/status/github/Xaymar/obs-stream-effects?branch=master&svg=true)](https://ci.appveyor.com/project/Xaymar/obs-stream-effects) |  [Linux, Mac: ![Travis](https://api.travis-ci.org/Xaymar/obs-stream-effects.svg?branch=master)](https://travis-ci.org/Xaymar/obs-stream-effects) | [Translations: ![Crowdin](https://d322cqt584bo4o.cloudfront.net/obs-stream-effects/localized.svg)](https://crowdin.com/project/obs-stream-effects)

# Support The Project
The project is funded through [all Supporters on Patreon](https://www.patreon.com/Xaymar), who get 30 day early access to releases, tutorials and more, and have a special Discord channel for priority support. They also (sometimes) have a say in what makes it into the plugin.

You can also help by [submitting translations through Crowdin](https://crowdin.com/project/obs-stream-effects)! Crowdin is a platform allowing anyone to submit translations for their favorite software (as long as that favorite software uses Crowdin), allowing more people to use the software without knowing another language.

# Features
## Source Mirror (Source)
Source Mirror duplicates the video and optionally also the audio of any source, allowing you to apply filters to either without affecting the original source. It also can rescale the video to fit your target region using any of the obs supported scaling modes. There is no limit on the number of Source Mirrors you create for one source, so go wild!

## Blur (Filter)
Blur out any unwanted content with this simple trick! This Filter allows you to apply various kinds of blur to any Source, even directional blur (often called motion blur)!

### Warning: Blurring is a non-destructive process, the source image can be restored to some degree. Do not use this to hide sensitive information, like clear-text passwords, bank information or credit card information. You have been warned.

## 3D Transform (Filter)
A popular request for OBS Studio is now available as a filter for you to use, move, rotate, scale and shear your Source in 3D space at will! Create reflections of your video camera on a floor, table, or place your video camera in a room - the possibilities are endless and you can choose what you want to do! It can even generate mipmaps for the transformed source to reduce aliasing on sharp angles or squished sources, to further improve quality.

## Inner/Outer Shadow (SDF) (Filter)
Add a shadow to any Source you wish to have a shadow for, optionally even a shadow in! Doesn't matter what kind of Source, just add this Filter and you're ready to have a Drop Shadow or an Inset Shadow, or even both!

## Displacement Mapping (Filter)
Displace the pixels of the Source, in any way you want - create a whirl, zoom in, whatever your input normal map can do will happen with this Filter.
