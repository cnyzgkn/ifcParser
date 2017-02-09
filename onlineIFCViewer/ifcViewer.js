var renderer;
//var stats;

function initThree() {
    width = document.getElementById('canvas-frame').clientWidth;
    height = document.getElementById('canvas-frame').clientHeight;
    renderer = new THREE.WebGLRenderer({
        antialias : true
    });
    renderer.setSize(width, height);
    document.getElementById('canvas-frame').appendChild(renderer.domElement);
    renderer.setClearColor(0x000000, 1.0);

    /*
    stats = new Stats();
    stats.domElement.style.position = 'absolute';
    stats.domElement.style.left = '0px';
    stats.domElement.style.top = '0px';
    document.getElementById('canvas-frame').appendChild(stats.domElement);
    */
}

var camera;
function initCamera() {
    camera = new THREE.PerspectiveCamera(45, width / height, 1, 1000000);
    camera.position.x = 30000;
    camera.position.y = 30000;
    camera.position.z = 30000;
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
    //ambient light
    light = new THREE.AmbientLight(0xFFFFFF);
    light.position.set(300000, 300000, 300000);
    scene.add(light);

    //sun light
    var directionalLight1 = new THREE.DirectionalLight(0xFFFFFF, 1);
    directionalLight1.position.set(1, 1.5, 1);
    this.scene.add(directionalLight1);

    var directionalLight2 = new THREE.DirectionalLight(0xFFFFFF, 1);
    directionalLight2.position.set(-1, -1.5, -1);
    this.scene.add(directionalLight2);
}

function initObjectfromJSONLowMemory() {
    $.ajaxSettings.async = false; 
    var fileNum = 1;
    var readJSON = true;
    while(readJSON == true)
    {
        var fileName = "MeshJiguiFinal/result" + fileNum.toString() + ".json";
        $.getJSON(fileName)
        .done(function(json){
            $.each(json, function(ifcObjectKey, ifcObject){ 
                if(ifcObject["noVertices"] !== 0)
                {
                    var geometry = new THREE.Geometry();
                    
                    var noVertices = ifcObject["noVertices"];
                    var vertices = ifcObject["vertices"];
                    var indicesForFaces = ifcObject["indicesForFaces"];
                    var noPrimitivesForFaces = ifcObject["noPrimitivesForFaces"];
                    //console.log("noPrimitivesForFaces == " + noPrimitivesForFaces);
                    //vertices
                    for(let i = 0; i < noVertices; i++)
                    {
                        let x = vertices[6*i+0];
                        let y = vertices[6*i+1];
                        let z = vertices[6*i+2];
                        geometry.vertices.push(new THREE.Vector3(
                            vertices[6*i+0],
                            vertices[6*i+1],
                            vertices[6*i+2])
                        );
                    }

                    //materials
                    var materials = [];
                    var ifcObjectMaterials = ifcObject["ifcObjectMaterials"];
                    var materialIndexArray = [];

                    var index = 0;
                    $.each(ifcObjectMaterials, function(ifcObjectMaterialKey, ifcObjectMaterial){
                        var materialIndexElement = 
                        {
                            materialIndex: index,
                            indexArrayPrimitives: ifcObjectMaterial.indexArrayPrimitives,
                            indexOffsetForFaces: ifcObjectMaterial.indexOffsetForFaces
                        };

                        /*
                        console.log("ifcObjectMaterial.materialValue.diffuse_R == " + ifcObjectMaterial.materialValue.diffuse_R);
                        console.log("ifcObjectMaterial.materialValue.diffuse_G == " + ifcObjectMaterial.materialValue.diffuse_G);
                        console.log("ifcObjectMaterial.materialValue.diffuse_B == " + ifcObjectMaterial.materialValue.diffuse_B);
                        console.log("ifcObjectMaterial.materialValue.specular_R == " + ifcObjectMaterial.materialValue.specular_R);
                        console.log("ifcObjectMaterial.materialValue.specular_G == " + ifcObjectMaterial.materialValue.specular_G);
                        console.log("ifcObjectMaterial.materialValue.specular_B == " + ifcObjectMaterial.materialValue.specular_B);
                        console.log("ifcObjectMaterial.materialValue.emissive_R == " + ifcObjectMaterial.materialValue.emissive_R);
                        console.log("ifcObjectMaterial.materialValue.emissive_G == " + ifcObjectMaterial.materialValue.emissive_G);
                        console.log("ifcObjectMaterial.materialValue.emissive_B == " + ifcObjectMaterial.materialValue.emissive_B);
                        */

                        
                        materials.push(new THREE.MeshPhongMaterial(
                        {
                            color: new THREE.Color(ifcObjectMaterial.materialValue.diffuse_R, ifcObjectMaterial.materialValue.diffuse_G, ifcObjectMaterial.materialValue.diffuse_B),
                            //color: 0xFFFFFF,
                            specular: new THREE.Color(ifcObjectMaterial.materialValue.specular_R, ifcObjectMaterial.materialValue.specular_G, ifcObjectMaterial.materialValue.specular_B),
                            emissive: new THREE.Color(ifcObjectMaterial.materialValue.emissive_R, ifcObjectMaterial.materialValue.emissive_G, ifcObjectMaterial.materialValue.emissive_B),
                            //shininess: ifcObjectMaterial.shininess, 
                            //transparency: ifcObjectMaterial.transparency==1.0 ? true : false,
                            shading: THREE.SmoothShading,
                            side: THREE.DoubleSide
                        }));

                        /*
                        materials.push(new THREE.MeshLambertMaterial(
                        {
                            color: new THREE.Color(ifcObjectMaterial.materialValue.diffuse_R, ifcObjectMaterial.materialValue.diffuse_G, ifcObjectMaterial.materialValue.diffuse_B),
                            specular: new THREE.Color(ifcObjectMaterial.materialValue.specular_R, ifcObjectMaterial.materialValue.specular_G, ifcObjectMaterial.materialValue.specular_B),
                            emissive: new THREE.Color(ifcObjectMaterial.materialValue.emissive_R, ifcObjectMaterial.materialValue.emissive_G, ifcObjectMaterial.materialValue.emissive_B),
                            //shininess: ifcObjectMaterial.shininess, 
                            //transparency: ifcObjectMaterial.transparency==1.0 ? true : false,
                            shading: THREE.SmoothShading,
                            side: THREE.DoubleSide
                        }));
                        */
                        
                        materialIndexArray.push(materialIndexElement);
                        ++index;
                    });

                    //console.log("materials.length == " + materials.length);

                    /*
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
                    */

                    
                    //faces
                    for(let i = 0; i < materials.length; i++)
                    {
                        var materialIndexElement = materialIndexArray[i];
                        /*
                        console.log("materialIndexElement materialIndex == " + materialIndexElement.materialIndex +
                            " indexArrayPrimitives " + materialIndexElement.indexArrayPrimitives 
                            + " indexOffsetForFaces " + materialIndexElement.indexOffsetForFaces);
                        console.log("max index == " + 3*(materialIndexElement.indexOffsetForFaces + materialIndexElement.indexArrayPrimitives -1) + 2);
                        */
                        for(let j = 0; j < materialIndexElement.indexArrayPrimitives; j++)
                        {
                            let k = materialIndexElement.indexOffsetForFaces + j*3;
                            //console.log("i == " + i + " j*3 == " + j*3 + " k == " + k);

                            var face = new THREE.Face3(
                                indicesForFaces[k+0],
                                indicesForFaces[k+1],
                                indicesForFaces[k+2]
                            );
                            face.materialIndex = materialIndexElement.materialIndex;
                            //console.log("i == " + i + " materialIndexElement.materialIndex == " + materialIndexElement.materialIndex);
                            geometry.faces.push(face);
                        }    
                        //console.log("\n\n");   
                    }

                    /*
                    //color
                    for ( var i = 0; i < geometry.faces.length; i++ ) {
                        geometry.faces[i].color.set(0xFF0000);
                    }
                    */

                    //calculate norm for lighting
                    geometry.computeFaceNormals();
                    geometry.computeVertexNormals();

                    var multiMaterials = new THREE.MultiMaterial(materials);
                    var mesh = new THREE.Mesh(geometry, multiMaterials);
                    mesh.position = new THREE.Vector3(0,0,0);
                    scene.add(mesh);   

                    //console.log("noPrimitivesForFaces == " + noPrimitivesForFaces);
                    //console.log("materials.length == " + materials.length);
                    //console.log("\n\n\n\n\n\n\n\n");    
                }
            });
            fileNum++;
            //console.log("fileNum == " + fileNum)   
        })
        .fail(function(jqXHR){
            //alert(fileName + " doesn't exist!");
            readJSON = false;
        });
    }
}

function threeStart() {
    initThree();
    initCamera();
    initScene();
    initLight();
    console.log("load json files success!");
    initObjectfromJSONLowMemory();
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
    //requestAnimationFrame(animation);
    //stats.update();
}

threeStart();

function testJSON() {
    $.ajaxSettings.async = false; 
    var num = 1;
    var readJSON = true;
    while(readJSON == true)
    {
        var fileName = "MeshJson/result" + num.toString() + ".json";
        $.getJSON(fileName)
        .done(function(json){
            $.each(json, function(ifcObjectIndex, ifcObject){ 
                document.writeln(ifcObject["globalId"]);
                //console.log(ifcObject["globalId"]);
            });
            num++;
        })
        .fail(function(jqXHR){
            //alert(fileName + " doesn't exist!");
            readJSON = false;
        });
    }
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