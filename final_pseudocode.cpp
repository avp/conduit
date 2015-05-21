// parameters of our algorithm, chosen based on qualitative testing

// angles in degrees
const int CROP_ANGLE = 180;
const int FOCUS_ANGLE = 30;

const int BLUR_FACTOR = 3;

OptimizedFrame optimize_frame(video_frame, orientation) {
  // note: video_frame is a fully-decoded video frame, i.e. just a simple
  // 2D array of RGB values

  // crop_frame crops the frame to your FOV, i.e. just the CROP_ANGLE degrees
  // you're looking at handling edge cases like if the viewer is looking
  // at the part of the panorama where the ends are stitched together.
  // h_angle = horizontal angle = how far left/right you're looking
  cropped_frame = crop(video_frame, orientation.h_angle, CROP_ANGLE);

  // reduces size by 1/BLUR_FACTOR in each dimension
  cropped_frame_blurred = scale(video_frame, 1/BLUR_FACTOR)

  // extract the part of the frame around the point you're looking at
  // of size FOCUS_ANGLE x FOCUS_ANGLE degrees
  focus_region = extract_region(
    video_frame,
    orientation.h_angle,
    orientation.v_angle,
    FOCUS_ANGLE
  );

  return OptimizedFrame(cropped_frame_blurred, focus_region, orientation);
}

Frame decode_frame(OptimizedFrame optimized) {
  cropped_frame = scale(optimized.cropped_frame_blurred, BLUR_FACTOR);
  orientation = optimized.orientation

  // uncrop the frame, essentially copying the cropped portion into
  // its original position on a new 360 degree frame (initially all black).
  frame = uncrop(cropped_frame, optimized.orientation.h_angle);

  // copy the focus region directly back onto the new frame in-place
  copy(frame, optimized.focus_region, orientation.h_angle, orientation.v_angle);

  return frame;
}

Video source_video;

Queue source_video_queue;
Queue optimized_queue;

Queue frame_queue; // uses an internal mutex
ViewerData viewer_data;
Mutex viewer_data_mutex;
Profiler profiler;

void video_decoder() {
  while (source_video.has_next_frame()) {
    frame = source_video.get_next_frame();
    source_videoframe_queue.enqueue(frame);
  }
}

void optimizer() {
  while (true) {
    // blocks until a frame is available
    frame = source_video_queue.dequeue();

    viewer_data_mutex.lock();
    local_viewer_data = viewer_data; // get a local copy
    viewer_data_mutex.unlock();

    // simulate server-client process

    // server would do this
    optimized_frame = optimize_frame(frame, local_viewer_data.orientation);

    // client would do this
    decoded_frame = decode_frame();

    // record the time the data used by a frame was received from the HMD
    decoded_frame.time = local_viewer_data.time;
    optimized_queue.enqueue(decoded_frame);
  }
}


int main() {
  // initialization of OpenGL, video, etc. omitted

  spawn_thread(video_decoder);
  spawn_thread(optimizer);

  OpenGLTexture texture; // reference a texture on the GPU

  while (true) {
    if (optimized_queue.size() > 0 || !texture.loaded) {
      // the below blocks until a frame is available, but except for the first
      // time, the if condition ensures there will be no wait.
      frame = optimized_queue.dequeue();
      profiler.add_sample(time() - frame.time); // how old the frame's data is

      // copies frame into a pixel-buffer object which is sent
      // non-blocking to the GPU
      copy_frame_to_texture(frame, texture);
    }

    viewer_data_mutex.lock();

    // estimate how long before the frame based on this data is displayed
    motion_to_update_time = profiler.average();

    // predicts the viewer data (orientation of HMD, etc.) at time given
    viewer_data = predict_viewer_data_from_hmd(time() + motion_to_update_time);
    viewer_data_mutex.unlock();

    // note: render actually draws the scene twice -- once for each eye --
    // the top half of the video frame (now in texture) is the left eye, and
    // the bottom half is the right eye
    render(texture, viewer_data);
  }
}
