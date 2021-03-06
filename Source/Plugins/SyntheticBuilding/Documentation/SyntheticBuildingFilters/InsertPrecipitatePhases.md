# Insert Precipitate Phases #

## Group (Subgroup) ##

Synthetic Building (Packing)

## Description ##

If there are precipitate _Phase Types_ in the volume, then this **Filter** will place precipitate **Features** with the sizes, shapes, physical orientations and locations corresponding to the goal statistics. Precipitate **Features** are currently placed _one at a time_ and are _not allowed to overlap_. The precpitiate packing process is similar to that for [packing primary phases](@ref packprimaryphases).

Currently, the parameters that are matched to target parameters include:

+ Fraction of precipitates on a grain boundary
+ Size distribution of the precipitates
+ Volume fraction of precipitates
+ Omega 3 distribution of precipitates in each size bin
+ b/a size distribution of precipitates in each size bin 
+ c/a size distribution of precipitates in each size bin 
+ Axis orientation distribution 
+ Normalized radial distribution function (if opted for by the user) 


Note that for an ellipsoid a > b > c.

The user can specify if they want *periodic boundary conditions*.  If they choose *periodic boundary conditions*, when the precipitate **Features** are being placed, if a **Feature** attempts to extend past the boundary of the volume it wraps to the opposing face and is placed on the opposite side of the volume.

The user can also specify if they want to write out the goal attributes of the generated precipitate **Features**.  The **Features**, once packed, will not necessarily have the exact statistics (size, shape, orientation, number of neighbors) as sampled from the distributions.  This is due to the use of non-space-filling objects in the packing process.  The overlaps and gaps that occur after packing, must be assigned and will cause the **Features** to deviate from the intended goal (albeit hopefully in a minor way).  Writing out the goal attributes allows the user to then calculate the actual attributes and compare to determine how well the packing algorithm is working for their **Features**.

The user can also choose to read in a list of precipitate **Features** with their locations and size and shape descriptions already determined. If this option is choosen, the **Filter** will skip the steps of generating the **Features** and iteratively placing them and will begin *growing* the **Features** defined in list.  The format of the *Feature Input File* is:

    Number of Features
    Phase X Y Z A B C O3 Phi1 PHI Phi2
    Phase X Y Z A B C O3 Phi1 PHI Phi2
    Phase X Y Z A B C O3 Phi1 PHI Phi2
    .
    .
    .
    Phase X Y Z A B C O3 Phi1 PHI Phi2

+ The file is a _SPACE_ delimited file
+ The first line of the file is an integer that is the number of features in the file.
+ All other lines contain 11 columns of data. *ALL* columns are required.

| Column | Type | Description |
| ------ | ---- | ----------- |
|  X     | Float | X Coordinate |
|  Y     | Float | Y Coordinate |
|  Z     | Float | Z Coordinate |
|  A     | Float | Major Pricipal Semi Axis Length |
|  B     | Float | Mid Pricipal Semi Axis Length |
|  C     | Float | Minor Pricipal Semi Axis Length |
| O3 | Float | Omega 3 Value of the feature |
| phi1 | Float | First Euler Angle |
| PHI  | Float | Second Euler Angle |
| phi2 | Float | Third Euler Angle |

where (X, Y, Z) are the coordinates of the **Feature's** centroid, (A, B, C) are the major, mid and minor principal semiaxis lengths of the **Feature**, O3 is the Omega 3 value of the **Feature** and (Phi1, PHI, Phi2) are the *Euler angles* that describe the **Features**'s orientation in the sample reference frame.

For more information on synthetic building, visit the [tutorial](@ref tutorialsyntheticsingle).

### How to write out the Goal Attributes ##

In previous versions of DREAM.3D for this filter there was an option to "Write Goal Attributes". This has been replaced with the feature that allows the user to save the Shape Parameters to another Attribute Matrix. To regain this functionality the user can enable this option from a drop down combo box in the Filter Parameter input widget, then add the "Export Feature Data as CSV File" later in the pipeline. In that added filter, the user can select the Feature Attribute Matrix where they saved the Shape Parameters.

## Parameters ##

| Name | Type | Description |
|------|------| ----------- |
| Periodic Boundaries | bool | Whether to *wrap* **Features** to create *periodic boundary conditions* |
| Match Radial Distribution Function | bool | Whether to attempt to match the _radial distribution function_ of the precipitates |
| Already Have Precipitates | bool | Whether to read in a file that lists the available precipitates |
| Precipitate Input File | File Path | The input precipitates file. Only needed if _Already Have Precipitates_ is checked |
| Write Goal Attributes | bool | Whether to write the goal attributes of the packed precipitates |
| Goal Attributes CSV File |  File Path | The output .csv file path. Only needed if _Write Goal Attributes_ is checked |
| Save Shape Description Arrays | Int | 0=Do not Save, 1=Save to New Attribute Matrix, 2=Append to existing AttributeMatrix |
| New AttributeMatrix | DataArrayPath | AttributeMatrix to save the Shape DescriptionArrays into |

## Required Geometry ##

Image

## Required Objects ##

| Kind | Default Name | Type | Component Dimensions | Description |
|------|--------------|------|----------------------|-------------|
| **Cell Attribute Array** | FeatureIds | int32_t | (1) | Specifies to which **Feature** each **Cell** belongs |
| **Cell Attribute Array**     | Phases            | int32_t | (1) | Specifies to which **Ensemble** each **Cell** belongs |
| **Cell Attribute Array** | BoundaryCells | int32_t | (1) | The number of neighboring **Cells** of a given **Cell** that belong to a different **Feature** than itself. Values will range from *0* to *6* |
| **Feature Attribute Array** | Phases | int32_t | (1) | Specifies to which **Ensemble** each **Feature** belongs |
| **Ensemble Attribute Array** | Statistics | Statistics Object | (1) | Statistics objects (depending on *Phase Type*) that store fits to descriptors such as size distribution, shape distribution, neighbor distribution, ODF, MDF, etc. |
| **Ensemble Attribute Array** | PhaseTypes | uint32_t | (1) | Enumeration specifying the phase type of each **Ensemble** |
| **Ensemble Attribute Array** | ShapeTypes | uint32_t | (1) | Enumeration specifying the type of shape to place for each **Ensemble** |
| **Ensemble Attribute Array** | NumFeatures | int32_t | (1) | Specifies the number of **Features** in each **Ensemble** |

## Created Objects ##

## Shape Description Arrays ##

| Kind | Default Name | Type | Component Dimensions | Description |
|------|--------------|------|----------------------|-------------|
| **Cell Attribute Array** | AxisEulerAngles | float | (3) | Morphological Orientation in Bunge Convention (ZX'Z'') |
| **Cell Attribute Array** | AxisLengths | float | (3) | The Ellipsoid Aspect Ratios[a, a/b, c/a] |
| **Cell Attribute Array** | Centroids | float | (3) | The XYZ value for each ellipsoid |
| **Cell Attribute Array** | EquivalentDiameters | float | (1) | Equivalent Spherical Diameter |
| **Cell Attribute Array** | Neighborhoods | int32_t | (1) |  |
| **Cell Attribute Array** | Omega3s | float | (1) | The Shape Parameter |
| **Cell Attribute Array** | Volumes | float | (1) | The Volume of the ellipsoid |

## Example Pipelines ##

+ (05) Composite
+ (04) Two Phase Cubic Hexagonal Particles Equiaxed

## License & Copyright ##

Please see the description file distributed with this **Plugin**

## DREAM.3D Mailing Lists ##

If you need more help with a **Filter**, please consider asking your question on the [DREAM.3D Users Google group!](https://groups.google.com/forum/?hl=en#!forum/dream3d-users)
