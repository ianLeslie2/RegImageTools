Functionally this project basically has two goals:
	1. Generate simplified versions of images where the image is represented as a set of uniformely colored connected regions
	2. Generate composition versions of images where the original image is represented through a set of smaller images
		To generate this "composition" the region-data from the previous step is used.
		Each connected region is used as a mask for a sub-image used in the composition.

To this end there are two main projects: ImageSimplify and ImageCompose that address the above listed points respectively.
In addition to this there is the dll-project "ImageInterface" which handles opening & closing images, as well as the static-lib
project "SharedLib" that contains various common classes.


The ".regdata" file format:
	Region data is stored in ".regdata" files.
	Currently all numeric values are stored as 4-byte unsigned integers
	
	Format specification for version 5:
		version-number
		uncompressed-block-size
		compressed-block-size
		<compressed-data-section> (compressed-block-size bytes long)
		
	The data-section is compressed using zLib
	When uncompressed, the data-section has the following format:
		region-count
		width
		height
		Region Colors: (region-count entries)
			red-val
			green-val
			blue-val
		Index List: (width*height entries. Each entry corresponds to a pixel listed left->right bottom->top)
			region-idx
	
	Regardless of any future changes, the first 4 bytes will always be the version number.
	
	--
	
	The RegData class in the SharedLib project handles this process of reading & writing the file format.

About ImageInterface:
	This project generates a dll called ImageInterface.dll with three methods:
		bool canProcess(const char* path)
		uint8_t* readData(const char* path, int* width, int* height)
		void writeData(const char* path, width, height, uint8_t* data)
	
	The methods are fairly self-explanatory.
	The pixel data is stored in an sRGB format in a uint8_t array
	Each pixel consists of the 4 values: red, green, blue, alpha
	Pixels are written left to right, bottom to top.
	
	This dll is dynamically loaded without any compile-time linkage.
	The idea is that the dll can be swapped out for something else if desired
	without needing to recompile any of the other projects.
	
	The currently supplied ImageInterface project relies on a local ImageMagick installation.
	The "magick" tool is used to convert files to and from a BMP format which can be interacted
	with directly.


About ImageSimplify:
	As mentioned previously, the general goal is to create a perceptually "simplified" version of the original image.
	
	Algorithm:
	----------
	In more technical terms, the goal/problem is:
		Given: An image and a positive integer N
		Produce: Produce an image of equal dimensions that consists of N uniformly colored connected regions, such that
					the "color" error is minimal
					
				Where "Color" error is defined as the sum of color differences between each input-output pixel pair
				The method of computing "color difference" I'm using is to convert to the LAB space and taking the euclidean distance
				between the color vectors.
	
	To achieve this I'm using a greedy algorithm that generates succesively simpler images.
	Pseudo-Code:
		For the input image consider each pixel it's own region
		curRegions = width*height
		while curRegions > N:
			Merge a pair of neighbouring regions such that the "color error" is minimal
				The color of new region is the average color of the pixels within it
	
	This algorithm doesn't generate the true error-minimal solution to the presented problem but the results are pretty good.
	
	Small Adjustment:
	-----------------
	The above algorithm isn't strictly true because the region-merge error function is written to slightly
	prefer merging smaller regions.
	Ultimately I'm after results that *look* nice to me and I prefered the results after this adjustment.
	(error function is getColorDiff(r1,r2) in Region.cpp)
	
	
	General Usage:
	--------------
	ImageSimplify <target-file> [optional arguments]
	
	If the file is name "testImage.jpg", then by default the tool will generate the following folder structure
	
	Output/
		testImage/
			regData/
				testImage_1.regdata
				testImage_2.regdata
				...
			ArgsUsedToGenerate.txt
			testImage_1.jpg
			testImage_2.jpg
			...
	
	So there's the general folder with the simplified images and regData sub-folder with the regdata files.
	ArgsUsedToGenerate.txt is a one-line file that just lists the last command used to generate the files.
		(This file only gets written to. Just something I found useful to keep track of things)
	
	Although the above example suggests that a file is output for every simplification level, that is not correct.
	The idea is to take a snapshot of each simplification stage that looks faily distinct. Snapshots get taken
	at region counts:
		1, 2, 3, 4, 5, 6, 7, 8, 9
		10,20,30,40,50,60,70,80,90
		100, 125, 150, 175 ... 500		(100->500 range breaks the pattern and outputs every 25)
		500, 600, 700, 800, 900
		1k, 2k, 3k, 4k, 5k, 6k, 7k, 8k, 9k
		etc.
	
	
	Note about target-file argument:
	--------------------------------
	You can also specify a .regdata file to start the simplification process from
	If the .regdata file-name has the format <something>_<number>.regdata and "outFilename" is not set
	then outFilename will be set to <something>
		(ie. You can target a previously generated regdata file without any issues)
	
	Optional arguments:
	-------------------
	
		outFilename
		-----------
		Base-name used for generating output files
		Output files are named:
			<base-name>_<regionCount>.<regdata OR jpg>
		
		By default this base-name is generated from the input file.
		
		outputFolder
		------------
		General folder for storing output. "Output" by default
		For each file processed, an output sub-folder is created.
		So if you process <something>.jpg, the results will be placed in <outputFolder>/<something>
		
		stopAt
		------
		The region-count to stop the simplification process at.
		The regdata will always be output for this region-count.
		1 by default
		
		outputAfter
		-----------
		Maxmimum region-count for which to generate files
		The simplification isn't particularly noticeable until there are less than 1000 regions
		so this is set to 1000 by default
		
		-regDataOnly (flag)
		------------
		If set, only the .regdata files will be generated
		
		scale
		-----
		This is a convienence option for running the algorithm on a lower-res verson of the image.
		With a scale of S, the input image is scaled down by a factor of S, processed, and then scaled up again.
		
		
		weightMap + weightMult
		----------------------
		Apart from trying to compute an "error minimal" simplification according to the above specification
		it can be interesting to personally exert some control over which areas of an image are simplified heavily or lightly.
		For example if you had an image of a person in front of a busy background you might want the person to be represented
		with more regions than the background since they are more important to the viewer.
		
		To this end there is an option to provide custom weights to the pixels in the target image.
		The "weight" of a pixel/region acts as a multiplier to the color-error that is calculated.
		(By default each pixel has a weight of 1 and each region a weight equal to the number of pixels in it)
		Intuitively, the "weight" of a region dictates how readily it will merge with neighboring regions.
		A region of the image with relatively high weight will be broken up into more regions.
		
		To specify these weight values two parameters are used. A greyscale* image referered to as a "weight-map" and a scale value
		refered to as "weightMult".
			weight(x,y) = 1 + weight-map(x,y).brightness * weightMult
		
		ie. A black pixel assigns a weight of 1, a white pixel assigns a weight of 1 + weightMult
		
		*The image does not technically have to be greyscale but it will internally be converted to greyscale before use
		
		
		contrastMap + contrastMult
		--------------------------
		Another control method available is adding a "contrast-map"
		The idea behind this option is to make certain regions of the image less likely to merge.
		
		As with the weight-map, a contrast-map image is supplied as well as a scaling value called contrastMult
		Internally the algorithm essentially runs on both two images (main-target + contrast-map) in parralel
		Only one set of regions is used, but each region tracks a color for the target image as well as the contrast-map
		and the color-error from both images is used to determine which regions are merged.


About ImageComp:
	
	ImageComp takes 3 arguments:
		1. A target image to be represent through a composition of sub-images
		2. A regdata file that determines the regions used for the composition
		3. A collection a sub-images to be used for the composition
	
	For each region of the target image, a sub-image is picked for which a "region-paste"
	is performed. A "region-paste" copies data from a sub-image into the pixels of a region.
	(ie. If the region is a circle a circular region of the sub-image is cut-out and pasted
		onto that area of the target-image)
	Similar to the simplify algorithm, a "color-error" function is used to determine which
	sub-image most closely matches the region.
	Here the "Color-Error" function used is the manhattan distance between colors in the RGB space.
	This is chosen due to it's simplicity and fact that the error for each color channel can be
	computed seperately.
	
	Although not explicitly mentioned above, when chosing a sub-image to "paste" an offset also
	needs to be consider.
	For example, lets say that we have a region with a bounding box of 100x100
	In order to cover the whole region cleanly we need an image that's at least 100x100
	However if we have an image thats 200x150 then there are additionally 100*50 = 5000 possible
	offsets we could use.
	
	For the sake of simplicity, each offset-image is considered a seperate image.
	So, how do we deal with image collections that can easily have over 10k items?
	Two methods are used:
		1. Instead of considering every possible offset we only consider certain increments
			(currently increments of 8)
		2. Statistical methods are used to narrow down the candidate image pool
	
	For method 2 images are broken up into 8x8 "tiles" and for each color-channel in each tile
	the normal distribution is tracked (mean & variance).
	When comparing two tiles we can operate on these distributions directly to quickly
	calculate a distribution for the expected color-error.
	And by adding up distributions we can quickly determine the expected color-error for two
	tiled images.
	Then we calculate the 80 percentile region of the distribution to determine a minimum and maximum
	probable error. Any image that has a min-error greater than the lowest max-error is discarded.
	
	A side-effect of method 2 is that the initial tile distributions for each library image only
	need to be calculated once and can be cached for future use.
	
	Once the potential image-set has been narrowed down, the error for the remaining images is manually calculated.
	