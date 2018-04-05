
  //var wsUri = "ws://localhost:1235";
  var wsUri = "ws://dev-aics-dtp-001:1235";

  var binarysocket0 = null; //handles requests for image streaming target #1
  //var binarysocket1 = null; //handles requests for image streaming target #2
  var jsonsocket0 = null;  //handles requests for dynamically populating the menu entries based on server feedback

  var dragFlag = 0; //for dragging in the render view
  var selectDragFlag = 0; //for dragging in the cell structure visibility widget
  var initialMouseX = 0;
  var initialMouseY = 0;
  var mouseSensi = 0.2;
  var img_width = 0;
  var img_height = 0;

  //quaternions
  var rotation;
  var oldRotation;
  var rotationDelta;
  var tempold;
  var slider_drag = false;

  var _stream_mode = false;
  var _stream_mode_suspended = false;


  /**
   * switches the supplied element to (in)visible
   * @param element
   * @param visible
   */
  function toggleDivVisibility(element, visible) {
      element.style.visibility = (visible ? "visible" : "hidden");
  }


var binarysock, jsonsock;
var gui;

function setupGui() {

  effectController = {
    resolution: "512x512",
    file: "//allen/aics/animated-cell/Allen-Cell-Explorer/Allen-Cell-Explorer_1.2.0/Cell-Viewer_Data/2017_05_15_tubulin/AICS-12/AICS-12_790.ome.tif",
    density: 50.0,
    exposure: 0.5,
    stream: false,
    skyTopColor: [255, 255, 255],
    skyMidColor: [255, 255, 255],
    skyBotColor: [255, 255, 255],
    lightColor: [255, 255, 255],
    lightIntensity: 100.0,
    lightDistance: 100.0,
    lightTheta: 0.0,
    lightPhi: 0.0,
    lightSize: 10.0
  };

  gui = new dat.GUI();
  //gui = new dat.GUI({autoPlace:false, width:200});

  gui.add(effectController, "file").onFinishChange(function(value) {
    var cb = new commandBuffer();
    cb.addCommand("LOAD_OME_TIF", value);
    flushCommandBuffer(cb);
    _stream_mode_suspended = true;
  });

  gui.add(effectController, "resolution", ["256x256", "512x512", "1024x1024", "1024x768"]).onChange(function(value) {
      var res = value.match(/(\d+)x(\d+)/);
      if (res.length === 3) {
        res[0] = parseInt(res[1]);
        res[1] = parseInt(res[2]);
        var imgholder = document.getElementById("imageA");
        imgholder.width = res[0];
        imgholder.height = res[1];
        imgholder.style.width = res[0];
        imgholder.style.height = res[1];

        var cb = new commandBuffer();
        cb.addCommand("SET_RESOLUTION", res[0], res[1]);
        flushCommandBuffer(cb);
      }
  });

  //allen/aics/animated-cell/Allen-Cell-Explorer/Allen-Cell-Explorer_1.2.0/Cell-Viewer_Data/2017_05_15_tubulin/AICS-12/AICS-12_790.ome.tif
  gui.add(effectController, "stream").onChange(function(value) {
    var cb = new commandBuffer();
    cb.addCommand("STREAM_MODE", value ? 1 : 0);
    flushCommandBuffer(cb);
    _stream_mode = value;
  });
  gui.add(effectController, "exposure").max(1.0).min(0.0).step(0.001).onChange(function(value) {
    var cb = new commandBuffer();
    cb.addCommand("EXPOSURE", value);
    flushCommandBuffer(cb);
    _stream_mode_suspended = true;
  }).onFinishChange(function(value) {
      _stream_mode_suspended = false;
  });
  gui.add(effectController, "density").max(100.0).min(0.0).step(0.001).onChange(function(value) {
        var cb = new commandBuffer();
        cb.addCommand("DENSITY", value);
        flushCommandBuffer(cb);
        _stream_mode_suspended = true;
    }).onFinishChange(function(value) {
        _stream_mode_suspended = false;
    });


    var lighting = gui.addFolder("Lighting");
    lighting.addColor(effectController, "skyTopColor").name("Sky Top").onChange(function(value) { 
        var cb = new commandBuffer();
        cb.addCommand("SKYLIGHT_TOP_COLOR", value[0]/255.0, value[1]/255.0, value[2]/255.0);
        flushCommandBuffer(cb);
        _stream_mode_suspended = true;
    }).onFinishChange(function(value) {
        _stream_mode_suspended = false;
    });
    lighting.addColor(effectController, "skyMidColor").name("Sky Mid").onChange(function(value) { 
        var cb = new commandBuffer();
        cb.addCommand("SKYLIGHT_MIDDLE_COLOR", value[0]/255.0, value[1]/255.0, value[2]/255.0);
        flushCommandBuffer(cb);
        _stream_mode_suspended = true;
    }).onFinishChange(function(value) {
        _stream_mode_suspended = false;
    });
    lighting.addColor(effectController, "skyBotColor").name("Sky Bottom").onChange(function(value) { 
        var cb = new commandBuffer();
        cb.addCommand("SKYLIGHT_BOTTOM_COLOR", value[0]/255.0, value[1]/255.0, value[2]/255.0);
        flushCommandBuffer(cb);
        _stream_mode_suspended = true;
    }).onFinishChange(function(value) {
        _stream_mode_suspended = false;
    });
    lighting.add(effectController, "lightDistance").max(100.0).min(0.0).step(0.1).onChange(function(value) {
        var cb = new commandBuffer();
        cb.addCommand("LIGHT_POS", 0, value, effectController["lightTheta"]*180.0/3.14159265, effectController["lightPhi"]*180.0/3.14159265);
        flushCommandBuffer(cb);
        _stream_mode_suspended = true;
    }).onFinishChange(function(value) {
        _stream_mode_suspended = false;
    });
    lighting.add(effectController, "lightTheta").max(180.0).min(-180.0).step(1).onChange(function(value) {
        var cb = new commandBuffer();
        cb.addCommand("LIGHT_POS", 0, effectController["lightDistance"], value*180.0/3.14159265, effectController["lightPhi"]*180.0/3.14159265);
        flushCommandBuffer(cb);
        _stream_mode_suspended = true;
    }).onFinishChange(function(value) {
        _stream_mode_suspended = false;
    });
    lighting.add(effectController, "lightPhi").max(180.0).min(0.0).step(1).onChange(function(value) {
        var cb = new commandBuffer();
        cb.addCommand("LIGHT_POS", 0, effectController["lightDistance"], effectController["lightTheta"]*180.0/3.14159265, value*180.0/3.14159265);
        flushCommandBuffer(cb);
        _stream_mode_suspended = true;
    }).onFinishChange(function(value) {
        _stream_mode_suspended = false;
    });
    lighting.add(effectController, "lightSize").max(100.0).min(0.01).step(0.1).onChange(function(value) {
        var cb = new commandBuffer();
        cb.addCommand("LIGHT_SIZE", 0, value, value);
        flushCommandBuffer(cb);
        _stream_mode_suspended = true;
    }).onFinishChange(function(value) {
        _stream_mode_suspended = false;
    });
    lighting.add(effectController, "lightIntensity").max(100.0).min(0.01).step(0.1).onChange(function(value) {
        var cb = new commandBuffer();
        cb.addCommand("LIGHT_COLOR", 0, effectController["lightColor"][0]/255.0*value, effectController["lightColor"][1]/255.0*value, effectController["lightColor"][2]/255.0*value);
        flushCommandBuffer(cb);
        _stream_mode_suspended = true;
    }).onFinishChange(function(value) {
        _stream_mode_suspended = false;
    });
    lighting.addColor(effectController, "lightColor").name("lightcolor").onChange(function(value) { 
        var cb = new commandBuffer();
        cb.addCommand("LIGHT_COLOR", 0, value[0]/255.0*effectController["lightIntensity"], value[1]/255.0*effectController["lightIntensity"], value[2]/255.0*effectController["lightIntensity"]);
        flushCommandBuffer(cb);
        _stream_mode_suspended = true;
    }).onFinishChange(function(value) {
        _stream_mode_suspended = false;
    });  


//  var customContainer = document.getElementById('my-gui-container');
//  customContainer.appendChild(gui.domElement);
}

dat.GUI.prototype.removeFolder = function(name) {
    var folder = this.__folders[name];
    if (!folder) {
      return;
    }
    folder.close();
    this.__ul.removeChild(folder.domElement.parentNode);
    delete this.__folders[name];
    this.onResize();
}
function onNewImage(infoObj) {

    // set up positions based on sizes.
    var x = infoObj.pixel_size_x * infoObj.x;
    var y = infoObj.pixel_size_y * infoObj.y;
    var z = infoObj.pixel_size_z * infoObj.z;
    var maxdim = Math.max(x,Math.max(y,z));
    gCamera.position.x = 0.5*x/maxdim;
    gCamera.position.y = 0.5*y/maxdim;
    gCamera.position.z = 1.5 + (0.5*z/maxdim);
    gCamera.up.x = 0.0;
    gCamera.up.y = 1.0;
    gCamera.up.z = 0.0;
    gControls.target.x = 0.5*x/maxdim;
    gControls.target.y = 0.5*y/maxdim;
    gControls.target.z = 0.5*z/maxdim;
    gControls.target0 = gControls.target.clone();


    setupChannelsGui(infoObj);
} 
function setupChannelsGui(infoObj) {
    if (effectController && effectController.infoObj) {
        for (var i = 0; i < effectController.infoObj.channel_names.length; ++i) {
            gui.removeFolder("Channel " + effectController.infoObj.channel_names[i]);
        }
    }

    effectController.infoObj = infoObj;
    infoObj.channelGui = [];
    for (var i = 0; i < infoObj.c; ++i) {
        infoObj.channelGui.push({
            colorD:[0,0,0],
            colorS:[0,0,0],
            colorE:[0,0,0],
            window: 1.0,
            level: 0.5,
            roughness: 0.0
        })
        var f = gui.addFolder("Channel " + infoObj.channel_names[i]);
        f.addColor(effectController.infoObj.channelGui[i], "colorD").name("Diffuse").onChange(function(j) { 
            return function(value) {
                    var cb = new commandBuffer();
                    cb.addCommand("MAT_DIFFUSE", j, value[0]/255.0, value[1]/255.0, value[2]/255.0, 1.0);
                    flushCommandBuffer(cb);
                };
        }(i));
        f.addColor(effectController.infoObj.channelGui[i], "colorS").name("Specular").onChange(function(j) {
            return function(value) {
                var cb = new commandBuffer();
                cb.addCommand("MAT_SPECULAR", j, value[0]/255.0, value[1]/255.0, value[2]/255.0, 1.0);
                flushCommandBuffer(cb);
            };
        }(i));
        f.addColor(effectController.infoObj.channelGui[i], "colorE").name("Emissive").onChange(function(j) {
            return function(value) {
                var cb = new commandBuffer();
                cb.addCommand("MAT_EMISSIVE", j, value[0]/255.0, value[1]/255.0, value[2]/255.0, 1.0);
                flushCommandBuffer(cb);
            };
        }(i));
        f.add(effectController.infoObj.channelGui[i], "window").max(1.0).min(0.0).step(0.001).onChange(function(j) {
            return function(value) {
                var cb = new commandBuffer();
                cb.addCommand("SET_WINDOW_LEVEL", j, value, effectController.infoObj.channelGui[j].level);
                flushCommandBuffer(cb);
                _stream_mode_suspended = true;
            }
        }(i))
        .onFinishChange(function(value) {
            _stream_mode_suspended = false;
        });

        f.add(effectController.infoObj.channelGui[i], "level").max(1.0).min(0.0).step(0.001).onChange(function(j) {
            return function(value) {
                var cb = new commandBuffer();
                cb.addCommand("SET_WINDOW_LEVEL", j, effectController.infoObj.channelGui[j].window, value);
                flushCommandBuffer(cb);
                _stream_mode_suspended = true;
            }
        }(i))
        .onFinishChange(function(value) {
            _stream_mode_suspended = false;
        });
        f.add(effectController.infoObj.channelGui[i], "roughness").max(100.0).min(0.0).onChange(function(j) {
            return function(value) {
                var cb = new commandBuffer();
                cb.addCommand("MAT_GLOSSINESS", j, value);
                flushCommandBuffer(cb);
                _stream_mode_suspended = true;
            }
        }(i))
        .onFinishChange(function(value) {
            _stream_mode_suspended = false;
        });
        
    }
    
}
  /**
   *
   */
  function init()
  {
    binarysocket0 = new WebSocket(wsUri); //handles requests for image streaming target #1
    binarysock = new binarysocket(0);
    binarysocket0.binaryType = "arraybuffer";
    //socket connection for image stream #1
    binarysocket0.onopen = binarysock.open;
    binarysocket0.onclose = binarysock.close;
    binarysocket0.onmessage = binarysock.message0; //linked to message0
    binarysocket0.onerror = binarysock.error;

//    jsonsocket0 = new WebSocket(wsUri); //handles requests for image streaming target #1
//    jsonsock = new jsonsocket();
//    jsonsocket0.binaryType = "arraybuffer";
    //socket connection for json message requests
//    jsonsocket0.onopen = jsonsock.open;
//    jsonsocket0.onclose = jsonsock.close;
//    jsonsocket0.onmessage = jsonsock.message;
//    jsonsocket0.onerror = jsonsock.error;

      //setup tooltips
    //readTextFile("data/tooltip.csv");

    var streamedImg = document.getElementsByClassName("streamed_img");

    // camera manipulations
    for(var i=0; i<streamedImg.length; i++)
    {
        streamedImg[i].addEventListener("wheel", MouseWheelHandler, false);
        streamedImg[i].addEventListener("mousedown", MouseDownHandler, false);
        streamedImg[i].addEventListener('ondragstart', DragStartHandler, false);
    }


    //set up first tab
    var streamimg1 = document.getElementById("imageA");

    toggleDivVisibility(streamimg1, true);

    setupGui();
  }



  /**
   * socket that exclusively receives binary data for streaming jpg images
   * @param channelnumber = 0 or 1 for left or right image => currently message0 or message1 are used since channelnumber cannot always be set via the constructor for some reason
   */
  function binarysocket(channelnumber = 0) {
    this.channelnum = channelnumber;
    this.open = function (evt) {
        //send the initial camera & data query upon opening the connection


        var cb = new commandBuffer();
        cb.addCommand("LOAD_OME_TIF", effectController.file);
        cb.addCommand("SET_RESOLUTION", 512, 512);
        cb.addCommand("FRAME_SCENE");
        //cb.addCommand("EYE", 0.5, 0.408, 2.145);
        //cb.addCommand("TARGET", 0.5, 0.408, 0.145);
        cb.addCommand("MAT_DIFFUSE", 0, 1.0, 0.0, 1.0, 1.0);
        cb.addCommand("MAT_SPECULAR", 0, 0.0, 0.0, 0.0, 0.0);
        cb.addCommand("MAT_EMISSIVE", 0, 0.0, 0.0, 0.0, 0.0);
        cb.addCommand("MAT_DIFFUSE", 1, 1.0, 1.0, 1.0, 1.0);
        cb.addCommand("MAT_SPECULAR", 1, 0.0, 0.0, 0.0, 0.0);
        cb.addCommand("MAT_EMISSIVE", 1, 0.0, 0.0, 0.0, 0.0);
        cb.addCommand("MAT_DIFFUSE", 2, 0.0, 1.0, 1.0, 1.0);
        cb.addCommand("MAT_SPECULAR", 2, 0.0, 0.0, 0.0, 0.0);
        cb.addCommand("MAT_EMISSIVE", 2, 0.0, 0.0, 0.0, 0.0);
        cb.addCommand("APERTURE", 0.0);
        cb.addCommand("EXPOSURE", 0.5);
        flushCommandBuffer(cb);

        // init camera
        var streamimg1 = document.getElementById("imageA");
        gCamera = new THREE.PerspectiveCamera(55.0, 1.0, 0.001, 20);
        gCamera.position.x = 0.5;
        gCamera.position.y = 0.5*0.675;
        gCamera.position.z = 1.5 + (0.5*0.133);
        gCamera.up.x = 0.0;
        gCamera.up.y = 1.0;
        gCamera.up.z = 0.0;
        gControls = new AICStrackballControls(gCamera, streamimg1);
        gControls.target.x = 0.5;
        gControls.target.y = 0.5*0.675;
        gControls.target.z = 0.5*0.133;
        gControls.target0 = gControls.target.clone();
        gControls.rotateSpeed = 4.0/window.devicePixelRatio;
        gControls.autoRotate = false;
        gControls.staticMoving = true;
        gControls.length = 10;
        gControls.enabled = true; //turn off mouse moments by setting to false

    };
    this.close = function (evt) {
        setTimeout(function () {
            //window.location.href = 'index.html';
            console.warn("connection failed. refresh to retry.");
        }, 3000);
        //document.write('Socket disconnected. Restarting..');
    };
    this.message = function (evt) {
        var bytes = new Uint8Array(evt.data),
            binary = "",
            len = bytes.byteLength,
            i;

        for (i=0; i<len; i++)
            binary += String.fromCharCode(bytes[i]);

        //console.log("msg received");
        screenImage.set(binary, this.channelnum);

    };

    this.message0 = function (evt) {
      console.time('recv');

      if (typeof(evt.data) === "string") {
          var returnedObj = JSON.parse(evt.data);
          if (returnedObj.commandId === COMMANDS.LOAD_OME_TIF[0]) {
              console.log(returnedObj);
              // set up gui!
              onNewImage(returnedObj);
            }
          return;
      }

      var bytes = new Uint8Array(evt.data),
        binary = "",
        len = bytes.byteLength,
        i;
      for (i=0; i<len; i++) {
        binary += String.fromCharCode(bytes[i]);
      }
      imgreceived = true;
      screenImage.set("data:image/png;base64,"+window.btoa( binary ), 0);
      console.timeEnd('recv');

      if (!_stream_mode_suspended && _stream_mode && !dragFlag) {
        // let cb = new commandBuffer();
        // cb.addCommand("REDRAW");
        // flushCommandBuffer(cb);
      }

// why should this code be slower?
      // var reader = new FileReader();
      // reader.onload = function(e) {
      //   imgreceived = true;
      //   screenImage.set(e.target.result, 0);
      //   console.timeEnd('recv');
      // };
      // reader.readAsDataURL(new Blob([new Uint8Array(evt.data)]));

    };

    this.error = function (evt) {
        console.log('error', evt);
    }
  }
  var lastevent;
  var filestructure = {};


  /**
   * socket that receives & handles json messages - used for setting up the client interface
   */
  function jsonsocket() {

    this.open = function (evt) {
        //console.log("opening json socket");

    };

    this.close = function (evt) {
        //setTimeout(function () { window.location.href = 'index.html'; }, 3000);
        //document.write('Socket disconnected. Restarting..');
        console.log('json socket closed', evt);
    };

    this.message = function (evt) {
        lastevent = evt;

        //parse incoming json
        filestructure = JSON.parse(evt.data);
        jsonfilestruct = filestructure;
    };

    this.error = function (evt) {
        console.log('error', evt);
    }
  }
  //todo: test if function is deprecated

  function send(msg)
  {
      this.mouseMoveTimer = setTimeout(function () {
        binarysocket0.send(msg);
      }.bind(this), 200);
  }

  var lastmsg;

  function flushCommandBuffer(cmdbuf) {
    var buf = cmdbuf.prebufferToBuffer();
    binarysocket0.send(buf);
  }

  /**
   * calls the "init" method upon page load
   */
  window.addEventListener("load", init, false);



