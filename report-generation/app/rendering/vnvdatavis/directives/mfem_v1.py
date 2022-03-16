def get_vert(x, y, shape):
    return x + y * shape


def get_elements(xdim, ydim, attr=1):
    elms = []
    for j in range(0, ydim-1):
        for i in range(0,xdim-1):
            elms.append(f"{attr} 3 {get_vert(i, j, xdim)} {get_vert(i + 1, j, xdim)} {get_vert(i + 1, j + 1, xdim)} {get_vert(i, j + 1, xdim)}")

    return elms


def get_boundary(xdim, ydim, attr=3):
    elms = []
    for i in range(0, xdim - 1):
        elms.append(f"{attr} 1 {get_vert(i, 0, xdim)} {get_vert(i + 1, 0, xdim)}")
        elms.append(f"{attr} 1 {get_vert(i, ydim-1, xdim)} {get_vert(i + 1, ydim-1, xdim)}")
    for j in range(0, ydim - 1):
        elms.append(f"{attr} 1 {get_vert(0, j, xdim)} {get_vert(0, j+1, xdim)}")
        elms.append(f"{attr} 1 {get_vert(xdim-1, j, xdim)} {get_vert(xdim-1, j+1, xdim)}")
    return elms


def get_vertices(xdim, ydim):
    res = []
    for j in range(0, ydim):
        for i in range(0, xdim):
            res.append(f"{i} {j}")
    return res


def sp(e):
    return "\n".join(e)


def toMesh(xdim, ydim, values):
    verts = get_vertices(xdim, ydim)
    elms = get_elements(xdim, ydim)
    bound = get_boundary(xdim, ydim)

    return f"""
MFEM mesh v1.0

dimension 
{2}

elements
{len(elms)}
{sp(elms)}

boundary
{len(bound)}
{sp(bound)}

vertices
{len(verts)}
2
{sp(verts)}
"""

if __name__ == "__main__":
    print(toMesh(5,10, None))
