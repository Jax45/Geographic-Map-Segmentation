# Geographic-Map-Segmentation
C++ Program that uses OpenCV to segment an image of the USA to get a specific state and zoom it in.

Jackson Hoenig

Computer Vision 6420

Programming Assignment 4

Date: 3/27/2021

Description: This program is made to segment out a specific state from the FullColorUSA.png image of the United States Of America.
	The program first takes in the input image as a command line argument, reads it, and displays it to the user.
	Next the user is able to click a state they want segmented. That click then extracts the color of the state and removes all
  other colors from a copy of the input image. Then that image expands a Region Of Interest(ROI) from that point until the next layer
  of expanding ROI contains no new pixels that are not black. That ROI is then doubled in pixels and superimposed on top of the same
  spot as the original state was in the input image. Note: If the resized state will go out of bounds of the input image it will be pushed away
  from the edge and a console message will detail that.
 
 # Example of clicking Missouri:
 
![Missouri ROI](https://github.com/Jax45/Geographic-Map-Segmentation/blob/master/Geographic%20Map%20Segmentation/MissouriROI.png?raw=true)

![Missouri Expanded](https://github.com/Jax45/Geographic-Map-Segmentation/blob/master/Geographic%20Map%20Segmentation/MissouriImgOut.png?raw=true)

# Example of clicking Michigan:

![Michigan ROI](https://github.com/Jax45/Geographic-Map-Segmentation/blob/master/Geographic%20Map%20Segmentation/MichiganROI.png?raw=true)

![Michigan Expanded](https://github.com/Jax45/Geographic-Map-Segmentation/blob/master/Geographic%20Map%20Segmentation/MichiganImgOut.png?raw=true)


# Known bugs:
When clicking a state near an edge of the screen the roi will increase until it hits that edge of the screen and then stop prematurely.

sometimes if you click a state like michigan too close to another state of the same color, it will increase the roi until it also hits the other state.
 this is needed though to pick up possible islands or disconnected parts of the state. it can possibly be refined to stop at a certain distance from the central region.

# Usage:

map_segment.exe [params] input_image

        -?, -h, --help, --usage
                print this message

        input_image (value:FullColorUSA.0.5.png)
                Name of file containing image
