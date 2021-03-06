Redcloud - REST Calls

**REST will not be used in future iterations of the project, except to moderate the devices from a web UI.  Its purpose is to facilitate defining the calls that will become standard invocations over other back-end communication methods (MQTT, WebSockets, etc.)


GET

	/on
	
		powers LED set on 
		
	/off
	
		powers LED set off
		
		
	/led_on
	
		powers on onboard LED light; used as a power / comm test
		
		
	/led_off
	
		powers off onboard LED light; used as a power / comm test
		
	
	/clear
	
		halts playback, flushes LED frame, turns all pixels off
		
	/reset 
		
	
		halts playback, flushes LED frame, resets the entire unit and executes setup()



POST 

	/set_frame (and variants)
	
		set_frame changes the state of the LED frame to whatever is specified by the frame request.  This state is persisted until another call to set_frame is made or
		another mode is invoked.  
		
		set_frame passes in the following:
		
				frame:  
					The set of pixels to change.  This is nominally the length of the LED frame, but it can be a subset of pixels beginning with the n-th pixel in the frame. 
				offset:   
					The number of pixels to skip before setting the frame, set to 0 by default.  
				operation:
					The type of operation to be performed against the existing LED frame state, set to overwrite by default :
					
							overwrite (default): replaces the existing LED frame with the new frame. 
							add_overlay : performs a logical color addition to the existing frame
							sub_overlay : performs a logical color substraction to the existing frame
							
				type:
					Type of color operation to use: RGB or HSV. This will determined the content of the frame itself 
							
				
		If a frame is passed in that is longer than than length - offset, the remaining values in that frame are simply omitted.  
		
		set_frame is good for buffered, continual operations, with frame content rendered prior to it being received by the node. 
		
		
	/set_mode (and variants)
			
		set_mode triggers a playback sequence over the LED frame.  The sequence plays indefinitely until another mode is selected or set_frame is invoked. 

		set_mode passes in the following:
		
				
				effect:
					The type of effect playback.  This would point to a function the performs the sequence over the LED frame.  
					
				params[]:
					An array of parameter values to be accepted by the effect function.  
					
				palette:
					The color palette to use by the effect. Optional.   
				
				frame:
					The frame that will seed the effect.  Optional, but if no frame is passed, the effect will start with the current frame set 
			
				rate:
					Playback rate, expressed in frames/sec.  [could correspond to a regularization parameter to control effect transition rate]
					
		
		set_mode is good for iterative playback, with frame content being generated by the node itself and does not require input.
					
	/map_frame 
	
		map_frame triggers an interpolation sequence between the current LED frame and the new frame.  The sequence plays until the new frame state is achieved, or if it
		is interrupted by another call.  
					
		
		map_frame passes in the following:
		
					
				frame:
					The target frame that will be attained at the end of the playback. 
					
				rate:
					The rate at which playback will take.  
					
				function:
					The type of mapping function to use to get from one point in the color space to another.  The function must be bijective. 
					
					
					
