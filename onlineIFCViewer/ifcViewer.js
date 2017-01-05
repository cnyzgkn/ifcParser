

var renderer;
var stats;

function initThree() {
    width = document.getElementById('canvas-frame').clientWidth;
    height = document.getElementById('canvas-frame').clientHeight;
    renderer = new THREE.WebGLRenderer({
        antialias : true
    });
    renderer.setSize(width, height);
    document.getElementById('canvas-frame').appendChild(renderer.domElement);
    renderer.setClearColor(0xFFFFFF, 1.0);

    stats = new Stats();
    stats.domElement.style.position = 'absolute';
    stats.domElement.style.left = '0px';
    stats.domElement.style.top = '0px';
    document.getElementById('canvas-frame').appendChild(stats.domElement);
}

var camera;
function initCamera() {
    camera = new THREE.PerspectiveCamera(45, width / height, 1, 1000000);
    camera.position.x = 20000;
    camera.position.y = 20000;
    camera.position.z = 20000;
    camera.up.x = 1;
    camera.up.y = 1;
    camera.up.z = 1;
    camera.lookAt({
        x : 0,
        y : 0,
        z : 0
    });
}

var scene;
function initScene() {
    scene = new THREE.Scene();
}

var light;
function initLight() {
    light = new THREE.AmbientLight(0xFF0000);
    light.position.set(100000, 100000, 100000);
    scene.add(light);
}

function initObjectfromJSON() {
    $.ajaxSettings.async = false; 
    $.getJSON("result.json",function(json){
        $.each(json, function(ifcObjectIndex, ifcObject){ 
            if(ifcObject["noVertices"] !== 0)
            {
                var geometry = new THREE.Geometry();
                
                var noVertices = ifcObject["noVertices"];
                var vertices = ifcObject["vertices"];
                var indicesForFaces = ifcObject["indicesForFaces"];
                var noPrimitivesForFaces = ifcObject["noPrimitivesForFaces"];
                //vertices
                for(let i = 0; i < noVertices; i++)
                {
                    let x = vertices[6*i+0];
                    let y = vertices[6*i+1];
                    let z = vertices[6*i+2];
                    var vertice = new THREE.Vector3(x, y, z);
                    geometry.vertices.push(vertice);
                }

                //faces
                for(let i = 0; i < noPrimitivesForFaces; i++)
                {
                    var face = new THREE.Face3(
                        indicesForFaces[3*i+0],
                        indicesForFaces[3*i+1],
                        indicesForFaces[3*i+2]
                        );
                    geometry.faces.push(face);
                }

                for ( var i = 0; i < geometry.faces.length; i++ ) {
                    geometry.faces[i].color.set(0xFF0000);
                }

                //calculate norm for lighting
                geometry.computeFaceNormals();
                geometry.computeVertexNormals();

                var material = new THREE.MeshBasicMaterial( { vertexColors: THREE.FaceColors} );
                var mesh = new THREE.Mesh( geometry,material);
                mesh.position = new THREE.Vector3(0,0,0);
                scene.add(mesh);
            }
        });
    });
}

function threeStart() {
    initThree();
    initCamera();
    initScene();
    initLight();
    initObjectfromJSON();
    animation();
}


function rotateFunction()
{
    mesh.rotateX(0.02);
    mesh.rotateY(0.02);
    mesh.rotateZ(0.02);
}

function rotateProperty()
{
    mesh.rotation.x += 0.01;
    mesh.rotation.y += 0.01;
    mesh.rotation.z += 0.01;
}


// 帧循环、游戏循环
function animation()
{
    renderer.render(scene, camera);
    requestAnimationFrame(animation);
    stats.update();
}

//threeStart();

function testJSON() {
    $.ajaxSettings.async = false; 
    $.getJSON("Jigui.json",function(json){
        $.each(json, function(ifcObjectIndex, ifcObject){ 
            document.writeln(ifcObject["globalId"])
        });
    });
}

//testJSON();

function testOboe() {
    oboe('JiguiFinal.json')
   .node('!.*', function(ifcObject){
      ifcObject = JSON.stringify(ifcObject);
      ifcObject = JSON.parse(ifcObject);
      document.writeln(ifcObject["globalId"]);
      return oboe.drop;
   }).done(function(){
      document.writeln("load complete!"); 
   })
}

//testOboe();

function testJSONStream() {
    //var JSONStream = require('JSONStream');

    fs.createReadStream('myfile.json')
    .pipe(JSONStream.parse('rows.*.doc'))
    .on('data', function(doc) {
    document.writeln[doc];
    });
}

//testJSONStream();