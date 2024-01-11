Входными данными для программы являются файлы сформированные программой Envi.
  Входные данные представляют собой два файла *.hdr и данные растрового изображения для
  различного количества каналов без расширения с тем же именем.

  https://www.l3harrisgeospatial.com/docs/ENVIHeaderFiles.html - описание файла *.hdr

##########################################################################################################################################################
#            Fields                      #              Description                                                    
##########################################################################################################################################################
1     acquisition time                   # Data acquisition time string that conforms to the ISO-8601 standard.
2     band names                         # The names of image bands.
3     bands                              # The number of bands per image file.
4     bbl                                # Lists the bad band multiplier values of each band in an image, typically 0 for bad bands and 1 for good bands.
5     byte order                         # Byte order=0 Host (Intel) least significant byte first (LSF)
6     class lookup                       # Lists class colors using RGB color definitions for classification files. For example, black is 0,0,0.
7     class names                        # Lists class names for classification files.
8     classes                            # Defines the number of classes, including unclassified regions, for classification files.
9     cloud cover                        # Percentage of cloud cover within the raster.
10    complex function                   # Specifies the values to calculate from a complex image and to use when displaying the image, calculating statistics for the image, or writing the image to a new file. Values include Real, Imaginary, Power, Magnitude, and Phase. The default value is Power.
11    coordinate system string           # It lists the parameters used for a geographic or projected coordinate system.
12    data gain values                   # Gain values for each band. Units are W/(m2 * µm * sr).
13    data ignore value                  # Pixel values that should be ignored in image processing.
14    data offset values                 # Offset values for each band.
15    data reflectance gain values       # An array of reflectance gain values.
16    data reflectance offset values     # An array of reflectance offset values. 
17    data type                          # 1 = Byte: 8-bit unsigned integer; 2 = Integer: 16-bit signed integer; 3 = Long: 32-bit signed integer; 4 = Floating-point: 32-bit single-precision; 5 = Double-precision: 64-bit double-precision floating-point; 6 = Complex: Real-imaginary pair of single-precision floating-point;9 = Double-precision complex: Real-imaginary pair of double precision floating-point;12 = Unsigned integer: 16-bit; 13 = Unsigned long integer: 32-bit; 14 = 64-bit long integer (signed); 15 = 64-bit unsigned long integer (unsigned)
18    default bands                      # Indicates which band numbers to automatically load Greyscale or R, G, and B
18    default stretch                    # The type of stretch to use when program displays the image.
19    dem band                           # Index (starting at 1) of a selected
20    dem file                           # Path and filename of a DEM associated with the image.
21    description                        # A string describing the image or the processing performed.
22    file type                          # The ENVI-defined file type, such as a certain data format and processing result.
23    fwhm                               # Lists full-width-half-maximum (FWHM) values of each band in an image.
24    geo points                         #
25    header offset                      #
26    interleave                         #
27    lines                              #
28    map info                           #
29    pixel size                         #
30    projection info                    #
31    read procedures                    #
32    reflectance scale factor           #
33    rpc info                           #
34    samples                            #
35    security tag                       #
36    sensor type                        #
37    solar irradiance                   #
38    spectra names                      #
39    sun azimuth                        #
40    sun elevation                      #
41    wavelength                         #
42    wavelength units                   #
43    x start                            #
44    y start                            #
45    z plot average                     #
46    z plot range                       #
47    z plot titles                      #