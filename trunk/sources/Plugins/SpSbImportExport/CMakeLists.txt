
# === CMake lists for the SoftPixel Scene Importer/Exporter - (08/07/2012) ===

set(SourceDir sources/Plugins/SpSbImportExport)

file(GLOB FilesSpSbImportExport ${SourceDir}/sps*)


# === Source group folders ===

source_group(
	Base FILES
	${SourceDir}/spsBaseDataTypes.h
	${SourceDir}/spsCore.h
	${SourceDir}/spsFileReader.cpp
	${SourceDir}/spsFileReader.h
	${SourceDir}/spsFileWriter.cpp
	${SourceDir}/spsFileWriter.h
)

source_group(
	FormatHandler FILES
	${SourceDir}/spsExporter.cpp
	${SourceDir}/spsExporter.h
	${SourceDir}/spsFormatEnumerations.h
	${SourceDir}/spsFormatHandler.cpp
	${SourceDir}/spsFormatHandler.h
	${SourceDir}/spsFormatMacros.h
	${SourceDir}/spsFormatStructures.h
	${SourceDir}/spsImporter.cpp
	${SourceDir}/spsImporter.h
)


# === Static library ===

add_library(SpSbImportExport STATIC ${FilesSpSbImportExport})
