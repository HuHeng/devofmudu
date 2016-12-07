#include <gst/gst.h>
#include <string.h>
#include <stdlib.h>

struct custom_data{
	GstElement *audio_queue;
	GstElement *video_queue;
};

static void cb_new_pad(GstElement *element, GstPad *pad, struct custom_data* data)
{
	gchar* name;
	name = gst_pad_get_name(pad);
	g_print("A new pad %s was created\n", name);
	g_free(name);
	GstPadLinkReturn ret;
	GstCaps *new_pad_caps = NULL;
	GstStructure *new_pad_struct = NULL;
	const gchar *new_pad_type = NULL;
	GstPad *audio_queue_sink_pad = NULL, *video_queue_sink_pad = NULL;
	/*setup a link for the new pad created*/
	/* Check the new pad's type */
	new_pad_caps = gst_pad_query_caps(pad, NULL);
	new_pad_struct = gst_caps_get_structure(new_pad_caps, 0);
	new_pad_type = gst_structure_get_name(new_pad_struct);
	if (g_str_has_prefix(new_pad_type, "audio")) {
		//link audio pad
		g_print("new_pad_type: %s\n", new_pad_type);
		audio_queue_sink_pad = gst_element_get_static_pad(data->audio_queue, "sink");
		if (gst_pad_is_linked(audio_queue_sink_pad)) {
			g_print("  We are already linked. Ignoring.\n");
			gst_object_unref(audio_queue_sink_pad);
			return;
		}
		if (gst_pad_link(pad, audio_queue_sink_pad) != GST_PAD_LINK_OK) {
			g_printerr("link failed! src pad addr: %p, audio sink pad addr: %p\n", pad, audio_queue_sink_pad);
		}	
		/*set queue: max-size-buffers=0 max-size-time=0 max-size-bytes=0 min-threshold-time=10000000000*/
		//(data->audio_queue, "max-size-buffers", 0, "max-size-time", 0, "max-size-bytes", 0, "min-threshold-time", 3000000000, NULL);
		g_object_set(data->audio_queue, "max-size-buffers", 0, "max-size-time", 0, "max-size-bytes", 0, "min-threshold-time", 3000000000, NULL);
		g_print("link audio ok!");
		gst_object_unref(audio_queue_sink_pad);
	}
	else if (g_str_has_prefix(new_pad_type, "video")) {
		//link video pad
		g_print("new_pad_type: %s\n", new_pad_type);
		video_queue_sink_pad = gst_element_get_static_pad(data->video_queue, "sink");
		if (gst_pad_is_linked(video_queue_sink_pad)) {
			g_print("  We are already linked. Ignoring.\n");
			gst_object_unref(video_queue_sink_pad);
			return;
		}
		if (gst_pad_link(pad, video_queue_sink_pad) != GST_PAD_LINK_OK) {
			g_printerr("link failed! src pad addr: %p, video sink pad addr: %p\n", pad, video_queue_sink_pad);
		}
		g_print("link video ok!\n");
		g_object_set(data->video_queue, "max-size-buffers", 0, "max-size-time", 0, "max-size-bytes", 0, "min-threshold-time", 3000000000, NULL);
		gst_object_unref(video_queue_sink_pad);
	}
	else {
		g_printerr("type was unknow: %s\n", new_pad_type);
	}
	//clean
	if (new_pad_caps != NULL)
		gst_caps_unref(new_pad_caps);
}

int main(int argc, char *argv[]) {
	/*if (argc != 2) {
		g_printerr("please input latency parameter!\n");
	}
	int latency = atoi(argv[1]);*/

	GstElement *pipeline, *source, *sink, *demuxer, *muxer, *audio_queue, *video_queue;
	GstBus *bus;
	GstMessage *msg;
	GstStateChangeReturn ret;
	//GstPad *audio_queue_sink_pad, *video_queue_sink_pad;
	GstPad *audio_queue_src_pad, *video_queue_src_pad;
	GstPad *demuxer_audio_src_pad, *demuxer_video_src_pad, *muxer_audio_sink_pad, *muxer_video_sink_pad;
	/* Initialize GStreamer */
	gst_init(&argc, &argv);

	//gst-launch-1.0 rtmpsrc location=rtmp://192.168.199.182/live/11 ! flvdemux name=mydemuxer flvmux name=mymuxer ! rtmpsink location=rtmp://192.168.199.182/live/22 mydemuxer.audio ! queue max-size-buffers=0 max-size-time=0 max-size-bytes=0 min-threshold-time=10000000000 ! mymuxer.audio  mydemuxer.video ! queue max-size-buffers=0 max-size-time=0 max-size-bytes=0 min-threshold-time=10000000000 ! mymuxer.video

	/* Create the elements */
	source = gst_element_factory_make("rtmpsrc", "source");
	sink = gst_element_factory_make("rtmpsink", "sink");
	demuxer = gst_element_factory_make("flvdemux", "demuxer");
	muxer = gst_element_factory_make("flvmux", "muxer");
	audio_queue = gst_element_factory_make("queue", "audio_queue");
	video_queue = gst_element_factory_make("queue", "video_queue");
	struct custom_data av_queue = {audio_queue, video_queue};
	/* Create the empty pipeline */
	pipeline = gst_pipeline_new("test-pipeline");

	if (!pipeline || !source || !sink || !demuxer || !muxer || !audio_queue || !video_queue) {
		g_printerr("Not all elements could be created.\n");
		return -1;
	}

	/* Build the pipeline */
	gst_bin_add_many(GST_BIN(pipeline), source, sink, demuxer, muxer, audio_queue, video_queue, NULL);
	if (gst_element_link(source, demuxer) != TRUE
		|| gst_element_link(muxer, sink) != TRUE) {
		g_printerr("Elements could not be linked.\n");
		gst_object_unref(pipeline);
		return -1;
	}
	//audio_queue_sink_pad = gst_element_get_static_pad(audio_queue, "sink");
	audio_queue_src_pad = gst_element_get_static_pad(audio_queue, "src");
	//video_queue_sink_pad = gst_element_get_static_pad(video_queue, "sink");
	video_queue_src_pad = gst_element_get_static_pad(video_queue, "src");
	//sometimes pad
	//demuxer_audio_src_pad = gst_element_get_request_pad(demuxer, "audio");
	//demuxer_video_src_pad = gst_element_get_request_pad(demuxer, "video");
	muxer_audio_sink_pad = gst_element_get_request_pad(muxer, "audio");
	muxer_video_sink_pad = gst_element_get_request_pad(muxer, "video");
	
	//link sometimes/dynamic pad
	//gst_element_link_pads(demuxer, "audio", audio_queue, "sink");
	//gst_element_link_pads(demuxer, "video", video_queue, "sink");


	if (//gst_pad_link(demuxer_audio_src_pad, audio_queue_sink_pad) != GST_PAD_LINK_OK ||
	//	 gst_pad_link(demuxer_video_src_pad, video_queue_sink_pad) != GST_PAD_LINK_OK ||
		 gst_pad_link(audio_queue_src_pad, muxer_audio_sink_pad) != GST_PAD_LINK_OK ||
		 gst_pad_link(video_queue_src_pad, muxer_video_sink_pad) != GST_PAD_LINK_OK) {
		g_printerr("pad could not be linked.\n");
		gst_object_unref(pipeline);
		return -1;
	}

	/* Modify the source's properties */
	g_object_set(source, "location", "rtmp://192.168.199.182/live/258", NULL);
	g_object_set(sink, "location", "rtmp://192.168.199.182/live/369", NULL);
	/*set queue: max-size-buffers=0 max-size-time=0 max-size-bytes=0 min-threshold-time=10000000000*/
	g_object_set(audio_queue, "max-size-buffers", 0, "max-size-time", 0, "max-size-bytes", 0, "min-threshold-time", 3000000000, NULL);
	g_object_set(video_queue, "max-size-buffers", 0, "max-size-time", 0, "max-size-bytes", 0, "min-threshold-time", 3000000000, NULL);
	
	g_signal_connect(demuxer, "pad-added", G_CALLBACK(cb_new_pad), &av_queue);

	/* Start playing */
	ret = gst_element_set_state(pipeline, GST_STATE_PLAYING);
	if (ret == GST_STATE_CHANGE_FAILURE) {
		g_printerr("Unable to set the pipeline to the playing state.\n");
		gst_object_unref(pipeline);
		return -1;
	}
	gboolean terminate = FALSE;
	/* Wait until error or EOS */
	bus = gst_element_get_bus(pipeline);
	do {
		msg = gst_bus_timed_pop_filtered(bus, GST_CLOCK_TIME_NONE,
			GST_MESSAGE_STATE_CHANGED | GST_MESSAGE_ERROR | GST_MESSAGE_EOS);

		/* Parse message */
		if (msg != NULL) {
			GError *err;
			gchar *debug_info;

			switch (GST_MESSAGE_TYPE(msg)) {
			case GST_MESSAGE_ERROR:
				gst_message_parse_error(msg, &err, &debug_info);
				g_printerr("Error received from element %s: %s\n", GST_OBJECT_NAME(msg->src), err->message);
				g_printerr("Debugging information: %s\n", debug_info ? debug_info : "none");
				g_clear_error(&err);
				g_free(debug_info);
				terminate = TRUE;
				break;
			case GST_MESSAGE_EOS:
				g_print("End-Of-Stream reached.\n");
				terminate = TRUE;
				break;
			case GST_MESSAGE_STATE_CHANGED:
				/* We are only interested in state-changed messages from the pipeline */
				if (GST_MESSAGE_SRC(msg) == GST_OBJECT(pipeline)) {
					GstState old_state, new_state, pending_state;
					gst_message_parse_state_changed(msg, &old_state, &new_state, &pending_state);
					g_print("Pipeline state changed from %s to %s:\n",
						gst_element_state_get_name(old_state), gst_element_state_get_name(new_state));
				}
				break;
			default:
				/* We should not reach here */
				g_printerr("Unexpected message received.\n");
				break;
			}
			gst_message_unref(msg);
		}
	} while (!terminate);

	/* Free resources */
	gst_object_unref(bus);
	gst_element_set_state(pipeline, GST_STATE_NULL);
	gst_object_unref(pipeline);
	return 0;
}