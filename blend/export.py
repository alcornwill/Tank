import bpy
from bpy.props import (
    StringProperty,
    BoolProperty
)
from bpy_extras.io_utils import ExportHelper, orientation_helper_factory, path_reference_mode

def write_C_array(out, _type, name, items):
    
    out.write('{} {}[] = {{\n    '.format(_type, name))
    count = 0
    for i in range(len(items)):
        row = items[i]
        lastrow = i == len(items) - 1
        for j in range(len(row)):
            item = row[j]
            
            count += 1
            if isinstance(item, float):
                out.write(' {0:.3f}'.format(item))
            else:
                out.write(' {}'.format(item))
            
            islast = j == len(row) - 1
            if lastrow and islast:
                out.write('\n')
            elif islast:
                out.write(',\n    ')
            else:
                out.write(',')
    out.write('};\n\n')
    return count

def custom_export(context, filepath, path_mode, use_edges, use_normals, use_colors, use_uvs):
    out = open(filepath, 'w')
    # get selected object
    obj = context.object 
    data = obj.data
    
    out.write('\n')
    out.write('#include <SDL_opengl.h>\n\n')
    
    indices = [list(poly.vertices) for poly in data.polygons]
    count = write_C_array(out, 'GLuint', 'indexData', indices)
    out.write('const int INDICES = {};\n\n'.format(count))   
       
    verts = [list(vert.co) for vert in data.vertices]
    count = write_C_array(out, 'GLfloat', 'vertexData', verts)
    out.write('const int VERTICES = {};\n\n'.format(count))
    
    if use_edges:
        edges = [list(edge.vertices) for edge in data.edges]
        count = write_C_array(out, 'GLuint', 'edgeData', edges)
        out.write('const int EDGES = {};\n\n'.format(count))    
    
    if use_normals:
        normals = [list(vert.normal) for vert in data.vertices]
        count = write_C_array(out, 'GLfloat', 'normalData', normals)
        out.write('const int NORMALS = {};\n\n'.format(count))
        
    if use_colors:
        colors = [list(color.color) for color in data.vertex_colors[0].data]
        count = write_C_array(out, 'GLfloat', 'colorData', colors)
        out.write('const int COLORS = {};\n\n'.format(count))
    
    if use_uvs:    
        uvs = [list(uv.uv) for uv in data.uv_layers[0].data]
        count = write_C_array(out, 'GLfloat', 'uvData', uvs)
        out.write('const int UVS = {};\n\n'.format(count))
            
    out.close()

IOOBJOrientationHelper = orientation_helper_factory("IOOBJOrientationHelper", axis_forward='-Z', axis_up='Y')

class SimpleOperator(bpy.types.Operator, ExportHelper, IOOBJOrientationHelper):
    """Tooltip"""
    bl_idname = "export_scene.custom"
    bl_label = "Custom Export"
    bl_options = {'REGISTER'}
    
    filename_ext = ".h"
    filter_glob = StringProperty(
        default="*.h",
        options={'HIDDEN'},
    )
    
    # no idea what this crap does
    path_mode = path_reference_mode
    check_extension = True
    
    edges = BoolProperty("Edges")
    normals = BoolProperty("Normals")
    colors = BoolProperty("Colors")
    uvs = BoolProperty("UVs")

    @classmethod
    def poll(cls, context):
        return context.active_object is not None

    def execute(self, context):
        custom_export(context, self.filepath, self.path_mode, self.edges, self.normals, self.colors, self.uvs)
        return {'FINISHED'}
    
def menu_func_export(self, context):
    self.layout.operator(SimpleOperator.bl_idname, text="Custom Export (.c)")


def register():
    bpy.utils.register_class(SimpleOperator)
    bpy.types.INFO_MT_file_export.append(menu_func_export)


def unregister():
    bpy.utils.unregister_class(SimpleOperator)
    bpy.types.INFO_MT_file_export.remove(menu_func_export)
    
if __name__ == "__main__":
    register()
