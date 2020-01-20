import agave

r = agave.renderer()
r.load_ome_tif("AICS-12_881_7.ome.tif")
r.render_iterations(100)
r.frame_scene()
r.session("test.png")
r.redraw()

