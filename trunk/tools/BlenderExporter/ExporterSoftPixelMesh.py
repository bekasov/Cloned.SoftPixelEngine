#
# SoftPixelMesh Exporter 1.1.1 for Blender 2.63 - (21/09/2011)
# Copyright (c) 2011 by Lukas Hermanns
#
# Contributors:
#    - "Mikey" (SoftPixel Forum Member) -> extended the script for Blender 2.63 to work with 'tessfaces'
#

#
# ======= Import libraries =======
#

import os
import struct
import math

try:
	import bpy
	import mathutils
except ImportError:
	sys.exit("Import libraries missing! Check that \"bpy\" and \"mathutils\" are installed.")

#
# ======= Blender exporter =======
#

bl_info = {
    "name": "SoftPixel Mesh Exporter (.spm)",
    "description": "Export all meshes in the scene to SoftPixel Mesh (.spm) files.",
    "author": "Lukas Hermanns",
    "version": (1, 1, 1),
    "blender": (2, 6, 3),
    "location": "File > Export > SoftPixel Mesh (.spm)",
    "warning": "",
    "wiki_url": "http://www.hlc-games.de/forum/viewtopic.php?f=12&t=752&sid=16b5e5100548784ff29e5dc6158554b2",
    "tracker_url": "http://www.hlc-games.de/forum/viewtopic.php?f=12&t=752&sid=16b5e5100548784ff29e5dc6158554b2",
    "category": "Import-Export"
}

if "bpy" in locals():
	import imp
	if "export_spm" in locals():
		imp.reload(export_spm)

from bpy.props import StringProperty, BoolProperty, EnumProperty
from bpy_extras.io_utils import (ExportHelper, axis_conversion, path_reference_mode)
from mathutils import Vector, Quaternion, Matrix

EPSILON = 0.0001

#
# ======= Classes =======
#

class OutputFileStream:
	"""This is the file output stream class"""
	
	stream = 0
	
	def writeFile(self, filename):
		self.stream = open(filename, 'wb')
	
	def closeFile(self):
		self.stream.close()
	
	def writeBuffer(self, type, value):
		self.stream.write(struct.pack(type, value))
	
	def writeByte(self, value):
		self.writeBuffer("b", value)
	
	def writeUByte(self, value):
		self.writeBuffer("B", value)
	
	def writeShort(self, value):
		self.writeBuffer("h", value)
	
	def writeUShort(self, value):
		self.writeBuffer("H", value)
	
	def writeInt(self, value):
		self.writeBuffer("i", value)
	
	def writeUInt(self, value):
		self.writeBuffer("I", value)
	
	def writeFloat(self, value):
		self.writeBuffer("f", value)
	
	def writeDouble(self, value):
		self.writeBuffer("d", value)
	
	def writeString(self, str):
		self.writeInt(len(str))
		for c in str:
			self.writeByte(ord(c))
	
	def writeStringN(self, str):
		for c in str:
			self.writeByte(ord(c))
		self.writeByte(ord('\n'))
	
	def writeColor(self, r = 255, g = 255, b = 255, a = 255):
		self.writeUByte(r)
		self.writeUByte(g)
		self.writeUByte(b)
		self.writeUByte(a)
	
	def writeVector(self, value, flipZ = True):
		self.writeFloat(value.x)
		self.writeFloat(value.y)
		
		if flipZ:
			self.writeFloat(-value.z)
		else:
			self.writeFloat(value.z)
	
	def writeMulVector(self, a, b):
		self.writeFloat(a.x * b.x)
		self.writeFloat(a.y * b.y)
		self.writeFloat(-a.z * b.z)
	
	def writeQuaternion(self, value):
		self.writeFloat(value.x)
		self.writeFloat(value.y)
		self.writeFloat(-value.z)
		self.writeFloat(value.w)

class JointVertexWeight:
	surface	= 0
	index	= 0
	weight	= 0.0
	
	def __init__(self, surface, index, weight):
		self.surface	= surface
		self.index		= index
		self.weight		= weight

class KeyframeTransformation:
	# Member variables
	position	= Vector([0.0, 0.0, 0.0])
	rotation	= Quaternion([1.0, 0.0, 0.0, 0.0])
	scale		= Vector([1.0, 1.0, 1.0])
	
	def __init__(self):
		position	= Vector([0.0, 0.0, 0.0])
		rotation	= Quaternion([1.0, 0.0, 0.0, 0.0])
		scale		= Vector([1.0, 1.0, 1.0])
	
	def write(self, file):
		file.writeVector(self.position)
		file.writeQuaternion(self.rotation)
		file.writeVector(self.scale, False)
	
	def extract(self, matrix):
		self.position	= matrix.to_translation()
		self.rotation	= matrix.to_quaternion()
		self.scale		= matrix.to_scale()
		
		self.rotation.normalize()

class JointKeyframe:
	# Member variables
	frame			= 0
	transformation	= None	# 'KeyframeTransformation' object
	
	def __init__(self, frame, transformation):
		self.frame			= frame
		self.transformation	= transformation

class AnimationJoint:
	"""This is the Animation Joint class for skeletal animation"""
	
	# Member variables
	name			= ''	# Jpint name
	index			= 0		# Joint index
	parent			= -1	# Parent joint index
	transformation	= None	# Base transformation
	keyframes		= None	# 'JointKeyframe' list
	vertices		= None	# 'JointVertexWeight' list
	
	def __init__(self):
		self.transformation = KeyframeTransformation()
		self.keyframes		= []
		self.vertices		= []

class SurfaceVertex:
	# Member variables
	surface	= 0		# Surface
	index	= 0		# Vertex index
	vertex	= None	# Vertex object
	
	def __init__(self, surface, index, vertex):
		self.surface	= surface
		self.index		= index
		self.vertex		= vertex

class SoftPixelMeshExporter:
	"""This is the SPM file exporter class"""
	
	# Member variables
	file			= OutputFileStream()
	context			= None
	surfaceVertices	= [];						# 'SurfaceVertex' list
	
	# Member constants
	MAX_COUNT_OF_TEXTURES			= 8
	
	SPM_VERSION_NUMBER				= 0x2000
	
	MDLSPM_CHUNK_NONE				= 0x0000
	
	MDLSPM_CHUNK_INDEX32BIT			= 0x0001
	MDLSPM_CHUNK_VERTEXCOLOR		= 0x0002
	MDLSPM_CHUNK_VERTEXFOG			= 0x0004
	MDLSPM_CHUNK_VERTEXNORMAL		= 0x0008
	
	MDLSPM_CHUNK_GOURAUDSHADING		= 0x0008
	MDLSPM_CHUNK_NODE_ANIM			= 0x0010
	MDLSPM_CHUNK_MORPHTARGET_ANIM	= 0x0020
	MDLSPM_CHUNK_SKELETAL_ANIM		= 0x0040
	
	MDLSPM_CHUNK_TEXTUREINTERN		= 0x0010
	MDLSPM_CHUNK_TEXTUREMATRIX		= 0x0020
	
	def __init__(self, context):
		self.context = context
	
	def writeHeader(self):
		file = self.file
		
		# Write identiy (SPMD)
		file.writeByte(ord('S'))
		file.writeByte(ord('P'))
		file.writeByte(ord('M'))
		file.writeByte(ord('D'))
		
		# Write version
		file.writeUShort(self.SPM_VERSION_NUMBER)
	
	def writeChunkVertex(self, face, vertex, texcoord, scale, surfIndex, vertIndex):
		file = self.file
		
		# Store vertex groups
		self.surfaceVertices.append(SurfaceVertex(surfIndex, vertIndex, vertex))
		
		# Vertex coordinate
		file.writeMulVector(vertex.co, scale)	# Coordinate
		
		# Texture coordinate
		if texcoord:
			file.writeFloat( texcoord[0])		# Texture coordinate X
			file.writeFloat(-texcoord[1])		# Texture coordinate Y
		else:
			file.writeFloat(0.0)				# Texture coordiante X
			file.writeFloat(0.0)				# Texture coordiante Y
		
		# Vertex normal
		if face.use_smooth:
			file.writeVector(vertex.normal)		# Smooth normal
		else:
			file.writeVector(face.normal)		# Flat normal
	
	def writeChunkFace(self, face, index):
		file = self.file
		
		# First triangle
		file.writeUShort(index + 2)		# Vertex index 0
		file.writeUShort(index + 1)		# Vertex index 1
		file.writeUShort(index + 0)		# Vertex index 2
		
		# Second triangle
		if len(face.vertices) == 4:
			file.writeUShort(index + 5)	# Vertex index 0
			file.writeUShort(index + 4)	# Vertex index 2
			file.writeUShort(index + 3)	# Vertex index 3
	
	def faceBelongsToSurface(self, face, images, surfIndex, imageName, hasNoneTexturedSurfaces):
		if not images:
			return True
		if surfIndex == 0 and ( not images[face.index] or not images[face.index].image ):
			return True
		if images[face.index].image.name == imageName and ( not hasNoneTexturedSurfaces or surfIndex > 0 ):
			return True
		return False
	
	def writeChunkSurface(self, obj, imageName, surfIndex, hasNoneTexturedSurfaces):
		file = self.file
		
		# First configuration
		hasTexture = len(imageName) > 0
		
		# Write surface identity and options
		if hasTexture:
			file.writeString(imageName)							# Name
		else:
			file.writeString("Surface" + str(surfIndex))		# Name
		
		file.writeShort(self.MDLSPM_CHUNK_VERTEXNORMAL)			# Flags
		
		# Write texture dimensions (only for one texture layer)
		file.writeUByte(2)										# First layer is 2 dimensional
		for i in range(1, self.MAX_COUNT_OF_TEXTURES):
			file.writeUByte(0)
		
		# Write each texture
		if hasTexture:
			file.writeUByte(1)									# Texture count
			file.writeUByte(1)									# Valid texture layer
			
			file.writeString(imageName)							# Texture filename
			file.writeUShort(0)									# Texture flags
			
			for i in range(3):
				file.writeInt(0)								# Unused options
		else:
			file.writeUByte(0)									# Texture count
		
		# Get texture coordinates
		images	= None
		tcoords	= None
		
		if obj.data.tessface_uv_textures and len(obj.data.tessface_uv_textures) > 0:
			tcoords = obj.data.tessface_uv_textures[0].data
			
			if tcoords:
				images = tcoords.values()
		
		# Get triangle count
		faces = obj.data.polygons
		
		triCount = 0
		
		for face in faces:
			# Check if face belongs to the current surface
			if self.faceBelongsToSurface(face, images, surfIndex, imageName, hasNoneTexturedSurfaces):
				if len(face.vertices) == 4:
					triCount += 2;
				else:
					triCount += 1;
		
		# Write each vertex
		verts = obj.data.vertices
		vertexCount = triCount * 3
		
		file.writeUInt(vertexCount)								# Vertex count
		
		if vertexCount > 0:
			# Write default vertex attributes
			file.writeColor()									# Vertex color
			file.writeFloat(0)									# Vertex fog coordinate
			
			vertIndex = 0
			
			for face in faces:
				# Check if face belongs to the current surface
				if not self.faceBelongsToSurface(face, images, surfIndex, imageName, hasNoneTexturedSurfaces):
					continue
				
				# Create vertices
				if tcoords and images:
					# Vertex coordinates (1st triangle)
					self.writeChunkVertex(face, verts[face.vertices[0]], tcoords[face.index].uv[0], obj.scale, surfIndex, vertIndex + 0)
					self.writeChunkVertex(face, verts[face.vertices[1]], tcoords[face.index].uv[1], obj.scale, surfIndex, vertIndex + 1)
					self.writeChunkVertex(face, verts[face.vertices[2]], tcoords[face.index].uv[2], obj.scale, surfIndex, vertIndex + 2)
					
					vertIndex += 3
					
					if len(face.vertices) == 4:
						# Vertex coordinates (2nd trianlge when face is a quad)
						self.writeChunkVertex(face, verts[face.vertices[0]], tcoords[face.index].uv[0], obj.scale, surfIndex, vertIndex + 0)
						self.writeChunkVertex(face, verts[face.vertices[2]], tcoords[face.index].uv[2], obj.scale, surfIndex, vertIndex + 1)
						self.writeChunkVertex(face, verts[face.vertices[3]], tcoords[face.index].uv[3], obj.scale, surfIndex, vertIndex + 2)
						
						vertIndex += 3
				else:
					# Vertex coordinates (1st triangle)
					self.writeChunkVertex(face, verts[face.vertices[0]], None, obj.scale, surfIndex, vertIndex + 0)
					self.writeChunkVertex(face, verts[face.vertices[1]], None, obj.scale, surfIndex, vertIndex + 1)
					self.writeChunkVertex(face, verts[face.vertices[2]], None, obj.scale, surfIndex, vertIndex + 2)
					
					vertIndex += 3
					
					if len(face.vertices) == 4:
						# Vertex coordinates (2nd trianlge when face is a quad)
						self.writeChunkVertex(face, verts[face.vertices[0]], None, obj.scale, surfIndex, vertIndex + 0)
						self.writeChunkVertex(face, verts[face.vertices[2]], None, obj.scale, surfIndex, vertIndex + 1)
						self.writeChunkVertex(face, verts[face.vertices[3]], None, obj.scale, surfIndex, vertIndex + 2)
						
						vertIndex += 3
		
		# Write each triangle
		file.writeUInt(triCount)								# Triangle count
		
		if len(faces) > 0:
			vertIndex = 0
			
			for face in faces:
				# Check if face belongs to the current surface
				if self.faceBelongsToSurface(face, images, surfIndex, imageName, hasNoneTexturedSurfaces):
					# Write face chunk
					self.writeChunkFace(face, vertIndex)
					
					if len(face.vertices) == 4:
						vertIndex = vertIndex + 6
					else:
						vertIndex = vertIndex + 3
	
	def writeChunkAnimationJoint(self, joint):
		file = self.file
		
		# Write joint basics
		file.writeString(joint.name)			# Joint name
		file.writeInt(joint.parent)
		
		# Write joint transformation
		joint.transformation.write(file)		# Joint transformation
		
		# Write vertex weights
		file.writeUInt(len(joint.vertices))		# Vertex count
		
		for vert in joint.vertices:
			file.writeUInt(vert.surface)		# Surface index
			file.writeUInt(vert.index)			# Vertex index
			file.writeFloat(vert.weight)		# Weight factor
		
		# Write keyframes:
		file.writeUInt(len(joint.keyframes))	# Keyframe count
		
		for keyframe in joint.keyframes:
			file.writeUInt(keyframe.frame)		# Frame index
			keyframe.transformation.write(file)	# Keyframe transformation
	
	def getArmature(self, obj):
		if obj.parent and obj.parent.type == 'ARMATURE':
			return obj.find_armature()
		return None
	
	def writeChunkAnimationSkeletal(self, obj):
		# Get armature
		armature = self.getArmature(obj)
		
		if not armature:
			return
		
		armatureAction = None
		if armature and armature.animation_data.action:
			armatureAction = armature.animation_data.action
		
		# Generate base transformation
		baseMatrix = mathutils.Matrix()
		baseMatrix.resize_4x4()
		baseMatrix[0] = [1, 0,  0, 0]
		baseMatrix[1] = [0, 0, -1, 0]
		baseMatrix[2] = [0, 1,  0, 0]
		baseMatrix[3] = [0, 0,  0, 1]
		
		# Get armature transformation matrix
		armatureMatrix = baseMatrix * armature.matrix_world
		
		# Write animation basics
		self.file.writeString(armature.name)					# Animation name
		
		# Get animation infos
		scene = self.context.scene
		
		firstFrame	= int(armatureAction.frame_range[0])
		lastFrame	= int(armatureAction.frame_range[1])
		frameCount	= (lastFrame - firstFrame) + 1
		
		prevFrame = scene.frame_current
		
		scene.frame_set(firstFrame)
		
		# Create joint list
		animJoints = []
		jointCount = 0
		
		for bone in armature.data.bones:
			# Create new animation joint
			animJoint = AnimationJoint()
			
			# Setup bone basics
			animJoint.name		= bone.name
			animJoint.index		= jointCount
			animJoint.parent	= -1
			
			# Setup bone transformation
			boneMatrix = Matrix(bone.matrix_local)
			
			if bone.parent:
				parentMatrix = Matrix(bone.parent.matrix_local).to_4x4()
				parentMatrix.invert()
				boneMatrix = parentMatrix * boneMatrix
			else:
				boneMatrix = armatureMatrix * boneMatrix
			
			animJoint.transformation.extract(boneMatrix)
			
			# Setup parnet bone index
			if bone.parent:
				otherBoneIndex = 0
				for otherBone in armature.data.bones:
					if bone.parent == otherBone:
						animJoint.parent = otherBoneIndex
						break
					otherBoneIndex += 1
			
			# Setup vertex weights
			for vert in self.surfaceVertices:
				for group in vert.vertex.groups:
					if obj.vertex_groups[group.group].name == bone.name and group.weight > EPSILON:
						animJoint.vertices.append(JointVertexWeight(vert.surface, vert.index, group.weight))
			
			# Add joint to the list
			animJoints.append(animJoint)
			jointCount += 1
		
		# Setup all keyframes
		frame = firstFrame
		
		while frame <= lastFrame:
			# Setup current scene frame
			scene.frame_set(frame)
			
			armatureMatrix = baseMatrix * armature.matrix_world
			
			# Setup current keyframe for each bone
			i = 0
			
			for bone in armature.data.bones:
				# Get animation joint
				animJoint = animJoints[i]
				i += 1
				
				# Setup bone transformation
				boneMatrix = armature.pose.bones[bone.name].matrix
				
				if bone.parent:
					parentMatrix = Matrix(armature.pose.bones[bone.parent.name].matrix).to_4x4()
					parentMatrix.invert()
					boneMatrix = parentMatrix * boneMatrix
				else:
					boneMatrix = armatureMatrix * boneMatrix
				
				# Get keyframe transformation
				transformation = KeyframeTransformation()
				transformation.extract(boneMatrix)
				
				# Store keyframe
				animJoint.keyframes.append(JointKeyframe(frame - firstFrame, transformation))
			
			frame += 1
		
		# Write animation joints
		self.file.writeUInt(jointCount)							# Joint count
		
		for joint in animJoints:
			self.writeChunkAnimationJoint(joint)
		
		# Reset editor settings
		scene.frame_set(prevFrame)
	
	def writeChunkObject(self, obj):
		# Write only one submesh
		self.file.writeInt(1)
		
		# Get flags
		flags = self.MDLSPM_CHUNK_GOURAUDSHADING
		
		if self.getArmature(obj):
			flags = flags | self.MDLSPM_CHUNK_SKELETAL_ANIM
		
		self.file.writeString(obj.name)							# Name
		self.file.writeShort(flags)								# Flags
		self.file.writeUInt(0)									# Reserved bytes (size for userdata)
		
		# Extract all textures
		#obj.update(calc_tessfaces=True)
		uvtexs = obj.data.tessface_uv_textures
		
		imageNames = []
		
		if uvtexs and len(uvtexs) > 0:
			images = uvtexs[0].data.values()
			
			for img in images:
				if img.image:
					imageNames.append(img.image.name)
			
			imageNames = list(set(imageNames))
		
		# Check if there are none textured surfaces
		hasNoneTexturedSurfaces = False
		
		'''
		for face in obj.data.faces:
			if face.
				hasNoneTexturedSurfaces = True
				break
		'''
		
		if len(imageNames) == 0:
			imageNames.append('')
			hasNoneTexturedSurfaces = True
		
		# Write each surface
		surfaceCount = len(imageNames)
		
		self.file.writeUInt(surfaceCount)
		
		for s in range(0, surfaceCount):
			self.writeChunkSurface(obj, imageNames[s], s, hasNoneTexturedSurfaces)
		
		# Write skeletal animation
		self.writeChunkAnimationSkeletal(obj)
	
	def exportMesh(self, obj, filename):
		# Open mesh file
		self.file.writeFile(filename)
		
		# Write header
		self.writeHeader()
		
		# Write object chunk
		self.writeChunkObject(obj)
		
		self.file.closeFile()

#
# ======= Global functions =======
#

def getValidFilename(filepath, name):
	# Get valid filename
	filename = filepath + '-' + name
	
	if os.path.isfile(filename + '.spm'):
		copyIndex = 1
		
		while os.path.isfile(filename + str(copyIndex) + '.spm'):
			copyIndex += 1
		
		filename = filename + str(copyIndex)
	
	return filename + '.spm'

def save(operator, context, filepath = "", use_selection = True):
	exporter = SoftPixelMeshExporter(context)
	
	# Write all objects from blender scene
	for obj in bpy.data.objects:
		if obj.type == 'MESH':
			if use_selection:
				if obj.select:
					exporter.exportMesh(obj, filepath)
					break
			else:
				exporter.exportMesh(obj, getValidFilename(filepath, obj.name))
	
	return {'FINISHED'}

#
# ======= Exporter menu =======
#

class ExportSPM(bpy.types.Operator, ExportHelper):
	# Export selection to SPM file (.spm)
	bl_idname = "export_scene.spm"
	bl_label = 'Export SPM'
	bl_options = {'PRESET'}
	
	filename_ext = ".spm"
	filter_glob = StringProperty(default = "*.spm", options = {'HIDDEN'})
	
	use_selection = BoolProperty(
		name = "First Selection Only",
		description = "Export first selected object only",
		default = True,
	)
	
	def execute(self, context):
		#from . import export_spm
		
		keywords = self.as_keywords(
			ignore = (
				"axis_forward",
				"axis_up",
				"check_existing",
				"filter_glob",
			)
		)
		
		return save(self, context, **keywords)

def menu_func_export(self, context):
    self.layout.operator(ExportSPM.bl_idname, text = "SoftPixel Mesh (.spm)")

def register():
	bpy.utils.register_module(__name__)
	bpy.types.INFO_MT_file_export.append(menu_func_export)

def unregister():
	bpy.utils.unregister_module(__name__)
	bpy.types.INFO_MT_file_export.remove(menu_func_export)
