At first we need to genearte test data to run these examples.

You may follow the instructions below to generate the Test input YUV files.

Go to https://media.xiph.org/video/derf/

Download 1080p versions of the following videos and put inside the **test_videos** directory.

**crowd_run_1080p50.y4m**

Convert the y4m files to 4:2:0 yuv format by using ffmpeg or any other video converter.

Sample command if using FFMpeg

```
ffmpeg -i crowd_run_1080p50.y4m crowd8_420_1080p50.yuv
```
