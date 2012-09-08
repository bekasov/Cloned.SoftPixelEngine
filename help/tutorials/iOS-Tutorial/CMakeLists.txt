
# === CMake lists for the iOS Tutorial - (08/09/2012) ===

set(TutPathISO ${TutorialPath}/iOS-Tutorial)

file(GLOB FilesSceneIOS		${TutPathISO}/Scene/*)
file(GLOB FilesClassesIOS	${TutPathISO}/Classes/*)
file(GLOB FilesAppIOS		${TutPathISO}/App/*)

add_executable(
	TutorialIOS
	MACOSX_BUNDLE
	${FilesSceneIOS}
	${FilesClassesIOS}
	${FilesAppIOS}
)

target_link_libraries(TutorialIOS SoftPixelEngine)
