/*
 *	@(#)TextureImage.java 1.20 00/03/31 14:28:06
 *
 * Copyright (c) 1996-2000 Sun Microsystems, Inc. All Rights Reserved.
 *
 * Sun grants you ("Licensee") a non-exclusive, royalty free, license to use,
 * modify and redistribute this software in source and binary code form,
 * provided that i) this copyright notice and license appear on all copies of
 * the software; and ii) Licensee does not utilize the software in a manner
 * which is disparaging to Sun.
 *
 * This software is provided "AS IS," without a warranty of any kind. ALL
 * EXPRESS OR IMPLIED CONDITIONS, REPRESENTATIONS AND WARRANTIES, INCLUDING ANY
 * IMPLIED WARRANTY OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE OR
 * NON-INFRINGEMENT, ARE HEREBY EXCLUDED. SUN AND ITS LICENSORS SHALL NOT BE
 * LIABLE FOR ANY DAMAGES SUFFERED BY LICENSEE AS A RESULT OF USING, MODIFYING
 * OR DISTRIBUTING THE SOFTWARE OR ITS DERIVATIVES. IN NO EVENT WILL SUN OR ITS
 * LICENSORS BE LIABLE FOR ANY LOST REVENUE, PROFIT OR DATA, OR FOR DIRECT,
 * INDIRECT, SPECIAL, CONSEQUENTIAL, INCIDENTAL OR PUNITIVE DAMAGES, HOWEVER
 * CAUSED AND REGARDLESS OF THE THEORY OF LIABILITY, ARISING OUT OF THE USE OF
 * OR INABILITY TO USE SOFTWARE, EVEN IF SUN HAS BEEN ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGES.
 *
 * This software is not designed or intended for use in on-line control of
 * aircraft, air traffic, aircraft navigation or aircraft communications; or in
 * the design, construction, operation or maintenance of any nuclear
 * facility. Licensee represents and warrants that it will not use or
 * redistribute the Software for such purposes.
 */

import java.applet.Applet;
import java.awt.*;
//import java.awt.event.*;
import com.sun.j3d.utils.applet.MainFrame;
import com.sun.j3d.utils.universe.*;
import com.sun.j3d.utils.image.TextureLoader;
import com.sun.j3d.utils.geometry.Box;
import javax.media.j3d.*;
import javax.vecmath.*;

public class TextureImage extends Applet {
  
    private java.net.URL texImage = null;

    public BranchGroup createSceneGraph() {
	// Create the root of the branch graph
	BranchGroup objRoot = new BranchGroup();

	// Create the transform group node and initialize it to the
	// identity.  Enable the TRANSFORM_WRITE capability so that
	// our behavior code can modify it at runtime.  Add it to the
	// root of the subgraph.
	TransformGroup objTrans = new TransformGroup();
	objTrans.setCapability(TransformGroup.ALLOW_TRANSFORM_WRITE);
	objRoot.addChild(objTrans);

	// Create appearance object for textured cube
	Appearance app = new Appearance();
	Texture tex = new TextureLoader(texImage, this).getTexture();
	app.setTexture(tex);
	TextureAttributes texAttr = new TextureAttributes();
	texAttr.setTextureMode(TextureAttributes.MODULATE);
	app.setTextureAttributes(texAttr);

	// Create textured cube and add it to the scene graph.
	Box textureCube = new Box(0.4f, 0.4f, 0.4f,
				  Box.GENERATE_TEXTURE_COORDS, app);
	objTrans.addChild(textureCube);

	// Create a new Behavior object that will perform the desired
	// operation on the specified transform object and add it into
	// the scene graph.
        double[] d = {10.f,10.f,10.f,10.f,
                      0.f,0.f,0.f,0.f,
                      0.f,0.f,0.f,0.f,
                      0.f,0.f,0.f,0.f};

	Transform3D yAxis = new Transform3D();
	Alpha rotationAlpha = new Alpha(-1, Alpha.INCREASING_ENABLE,
					0, 0,
					4000, 0, 0,
					0, 0, 0);

	RotationInterpolator rotator =
	    new RotationInterpolator(rotationAlpha, objTrans, yAxis,
				     0.0f, (float) Math.PI*2.0f);
	BoundingSphere bounds =
	    new BoundingSphere(new Point3d(-100.1f,0.0,0.0), 100.0);
	rotator.setSchedulingBounds(bounds);
        objTrans.setBounds(new BoundingSphere(new Point3d(-100.0,0.0,0.0),2.0));
	objTrans.addChild(rotator);


        // Have Java 3D perform optimizations on this scene graph.
        objRoot.compile();

	return objRoot;
    }

    public TextureImage() {
    }

    public TextureImage(java.net.URL url) {
        texImage = url;
    }

    public void init() {
        if (texImage == null) {
  	    // the path to the image for an applet
  	    try {
	        texImage = new java.net.URL(getCodeBase().toString() +"G:\\work\\lab\\java3d\\stone.jpg");
            System.out.println(getCodeBase().toString() +"stone.jpg");
	    }
	    catch (java.net.MalformedURLException ex) {
	        System.out.println(ex.getMessage());
		System.exit(1);
	    }
	}
	setLayout(new BorderLayout());
        GraphicsConfiguration config =
           SimpleUniverse.getPreferredConfiguration();

        Canvas3D c = new Canvas3D(config);
	add("Center", c);

	// Create a simple scene and attach it to the virtual universe
	BranchGroup scene = createSceneGraph();
	SimpleUniverse u = new SimpleUniverse(c);

        // This will move the ViewPlatform back a bit so the
        // objects in the scene can be viewed.
	u.getViewingPlatform().setNominalViewingTransform();

	u.addBranchGraph(scene);
    }

    //
    // The following allows TextureImage to be run as an application
    // as well as an applet
    //
    public static void main(String[] args) {
        java.net.URL url = null;
        if (args.length > 0) {
	    try {
	        url = new java.net.URL("file:" + args[0]);
	    }
	    catch (java.net.MalformedURLException ex) {
	        System.out.println(ex.getMessage());
		System.exit(1);
	    }
	}
	else {
	    // the path to the image for an application
	    try {
	        url = new java.net.URL("file:G:\\work\\lab\\java3d\\stone.jpg");
	    }
	    catch (java.net.MalformedURLException ex) {
	        System.out.println(ex.getMessage());
		System.exit(1);
	    }
	}
	new MainFrame(new TextureImage(url), 600, 600);
    }

}
