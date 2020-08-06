import 'vtk.js/Sources/favicon';

import macro from 'vtk.js/Sources/macro';
import vtkFullScreenRenderWindow from 'vtk.js/Sources/Rendering/Misc/FullScreenRenderWindow';

import vtkActor from 'vtk.js/Sources/Rendering/Core/Actor';
import vtkCalculator from 'vtk.js/Sources/Filters/General/Calculator';
import vtkDataSet from 'vtk.js/Sources/Common/DataModel/DataSet';
import vtkLookupTable from 'vtk.js/Sources/Common/Core/LookupTable';
import vtkMapper from 'vtk.js/Sources/Rendering/Core/Mapper';
import vtkPlaneSource from 'vtk.js/Sources/Filters/Sources/PlaneSource';
import vtkPoints from 'vtk.js/Sources/Common/Core/Points';
import vtkPolyData from 'vtk.js/Sources/Common/DataModel/PolyData';
import vtkWarpScalar from 'vtk.js/Sources/Filters/General/WarpScalar';


const { ColorMode, ScalarMode } = vtkMapper;
const { FieldDataTypes } = vtkDataSet;
const { vtkErrorMacro } = macro;


function emptyContainer(container) {
  while (container.firstChild) {
    container.removeChild(container.firstChild);
  }
}

function createViewer(rootContainer, form, xres, yres, scalar, vis  ) {

  // ----------------------------------------------------------------------------
  // Standard rendering code setup
  // ----------------------------------------------------------------------------
  const options = Object.assign({ containerStyle: { height: '100%' } });
  const background = [0.9, 0.9, 0.9];
  const containerStyle = options.containerStyle;
  const fullScreenRenderer = vtkFullScreenRenderWindow.newInstance({background, rootContainer, containerStyle});
  const renderer = fullScreenRenderer.getRenderer();
  const renderWindow = fullScreenRenderer.getRenderWindow();
  const lookupTable = vtkLookupTable.newInstance({ hueRange: [0.666, 0] });

  const planeSource = vtkPlaneSource.newInstance({
    xResolution: xres,
    yResolution: yres,
  });

  const planeMapper = vtkMapper.newInstance({
    interpolateScalarsBeforeMapping: true,
    colorMode: ColorMode.DEFAULT,
    scalarMode: ScalarMode.DEFAULT,
    useLookupTableScalarRange: true,
    lookupTable,
  });
  const planeActor = vtkActor.newInstance();
  planeActor.getProperty().setEdgeVisibility(true);

  const simpleFilter = vtkCalculator.newInstance();
  simpleFilter.setFormulaSimple(
    FieldDataTypes.POINT, // Generate an output array defined over points.
    [], // We don't request any point-data arrays because point coordinates are made available by default.
    'z', // Name the output array "z"
    (x) => (x[0] - 0.5) * (x[0] - 0.5) + (x[1] - 0.5) * (x[1] - 0.5) + 0.125
   ); // Our formula for z

  const warpScalar = vtkWarpScalar.newInstance();
  const warpMapper = vtkMapper.newInstance({
    interpolateScalarsBeforeMapping: true,
    useLookupTableScalarRange: true,
    lookupTable,
  });
  const warpActor = vtkActor.newInstance();

  // The generated 'z' array will become the default scalars, so the plane mapper will color by 'z':
  simpleFilter.setInputConnection(planeSource.getOutputPort());

  // We will also generate a surface whose points are displaced from the plane by 'z':
  warpScalar.setInputConnection(simpleFilter.getOutputPort());
  warpScalar.setInputArrayToProcess(0, 'z', 'PointData', 'Scalars');

  planeMapper.setInputConnection(simpleFilter.getOutputPort());
  planeActor.setMapper(planeMapper);
  warpScalar.set({ ['scaleFactor']: scalar });
  planeActor.setVisibility(vis);

  warpMapper.setInputConnection(warpScalar.getOutputPort());
  warpActor.setMapper(warpMapper);

  renderer.addActor(planeActor);
  renderer.addActor(warpActor);
  renderer.resetCamera();

  let fn = null;
  try {
    /* eslint-disable no-new-func */
    fn = new Function('x,y', `return ${form}`);
    /* eslint-enable no-new-func */
  } catch (exc) {
    if (!('name' in exc && exc.name === 'SyntaxError')) {
      vtkErrorMacro(`Unexpected exception ${exc}`);
      return;
    }
  }
  if (fn) {
    const formulaObj = simpleFilter.createSimpleFormulaObject(
      FieldDataTypes.POINT,
      [],
      'z',
      fn
    );

    // See if the formula is actually valid by invoking "formulaObj" on
    // a dataset containing a single point.
    planeSource.update();
    const arraySpec = formulaObj.getArrays(planeSource.getOutputData());
    const testData = vtkPolyData.newInstance();
    const testPts = vtkPoints.newInstance({
      name: 'coords',
      numberOfComponents: 3,
      size: 3,
      values: [0, 0, 0],
    });
    testData.setPoints(testPts);
    const testOut = vtkPolyData.newInstance();
    testOut.shallowCopy(testData);
    const testArrays = simpleFilter.prepareArrays(arraySpec, testData, testOut);
    try {
      formulaObj.evaluate(testArrays.arraysIn, testArrays.arraysOut);

      // We evaluated 1 point without exception... it's safe to update the
      // filter and re-render.
      simpleFilter.setFormula(formulaObj);

      simpleFilter.update();

      // Update UI with new range
      const [min, max] = simpleFilter
        .getOutputData()
        .getPointData()
        .getScalars()
        .getRange();
      lookupTable.setMappingRange(min, max);

      renderWindow.render();
      return;
    } catch (exc) {
      vtkErrorMacro(`Unexpected exception ${exc}`);
    }
  }
}

export function ready(fn) {
   if (document.readyState != 'loading') {
     fn();
   } else {
     document.addEventListener('DOMContentLoaded',fn);
   }
}

ready(function() {
  const viewerContainers = document.querySelectorAll('.vtkjs-function-viewer');
  let nbViewers = viewerContainers.length;
  while (nbViewers--) {
    const viewerContainer = viewerContainers[nbViewers];
    console.log(viewerContainer.dataset)
    const func = viewerContainer.dataset.func || 'x[0]*x[1]' 
    const res = Number(viewerContainer.dataset.res) || 230 
    const sca = Number(viewerContainer.dataset.scale) || 1 
    const vis = !(viewerContainer.hasAttribute("data-noplane"))  	    
    console.log(func)
    console.log(res)
    console.log(sca)
    console.log(vis)
    createViewer(viewerContainer,func,res,res,sca,vis)
  }
}
)

