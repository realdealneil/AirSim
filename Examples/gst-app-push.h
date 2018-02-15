/***********************************************************************
 * gst-app-push.h
 * 
 * Author: Neil Johnson
 * 
 * Purpose: A header-only class that just needs you to add a callback 
 *   that pushes image buffers in order to start streaming data.  
 * 
 **********************************************************************/

#include <gst/gst.h>
#include <gst/app/gstappsrc.h>

#include <cstdio>
#include <cstring>
#include <cstdlib>
#include <stdint.h>

#include <iostream>

#include <thread>

#include <opencv2/core/core.hpp>

#define DEBUGME 0

//uint8_t b = 127;

class GstAppPush
{
public:
	struct configOpts {
		int iHeight;
		int iWidth;
		int bitRate;
		int numChan;
		std::string formatStr;
		
		configOpts() 
			: iWidth(244)
			, iHeight(156)
			, numChan(1)
			, formatStr("GRAY8")
			, bitRate(1000000)
		{}
	};	
	
private:
	GstElement *pipeline;
	GstElement *appsrc;
	GstBus *bus;
	guint hid;
	std::thread gstThread;
	
	bool open;
	
public:
	GMainLoop *loop;
	configOpts c_;	
	cv::Mat img_;
	bool imageUpdated;
	
	GstAppPush() 
		: open(false)
	{
		
	}
	
	~GstAppPush()
	{
		Close();
	}
	
	void Close()
	{
		if (open) {
			gst_element_send_event(pipeline, gst_event_new_eos());
			gstThread.join();
			open = false;
		}
	}
	
	void PushImage(const cv::Mat &img) {
		img_ = img;
		imageUpdated = true;
	}
	
	static gboolean	read_data (GstAppPush * app)
	{		
		static int count = 0;
		
		if (!app->imageUpdated)
		{
			return TRUE;
		}
		
		int w = app->img_.cols; 
		int h = app->img_.rows; 
		int c = app->img_.channels();
		
		int imageSizeBytes = w*h*c*sizeof(uint8_t);

		GstBuffer *buffer = gst_buffer_new_wrapped_full(
				(GstMemoryFlags)0,
				(gpointer)app->img_.data,
				imageSizeBytes,
				0,
				imageSizeBytes,
				NULL,
				NULL);
				
		GstFlowReturn ret;
		g_signal_emit_by_name (app->appsrc, "push-buffer", buffer, &ret);
		gst_buffer_unref (buffer);
		
		gboolean ok = TRUE;

		if (ret != GST_FLOW_OK) {
			/* some error, stop sending data */
			GST_DEBUG ("some error");
			ok = FALSE;
		}	
		
#if DEBUGME
		printf("read_data called %d times\n", ++count);
#endif
		
		app->imageUpdated = false;

		return ok;
	}
	
	/**
	 * 	Start the gstreamer main loop:
	 */
	int Start(const configOpts &c) 
	{
		c_ = c;		
		
		//! Initialize main loop:
		gst_init(NULL, NULL);
		loop = g_main_loop_new (NULL, TRUE);
		
		//! Create pipeline: 
		pipeline = gst_parse_launch(
			"appsrc name = mysource ! "
			"  queue ! "
			"  autovideosink",
			NULL);
			
		g_assert(pipeline);
		
		//! Create bus to watch for messages on pipeline:
		bus = gst_pipeline_get_bus (GST_PIPELINE (pipeline));
		g_assert(bus);
		gst_bus_add_watch (bus, (GstBusFunc) bus_message, this);
		
		//! Create the appsrc:
		appsrc = gst_bin_get_by_name(GST_BIN(pipeline), "mysource");
		g_assert(appsrc);
		g_assert(GST_IS_APP_SRC(appsrc));
		
		g_object_set(G_OBJECT(appsrc), "caps",
			gst_caps_new_simple(
				"video/x-raw", 
				"format", G_TYPE_STRING, c_.formatStr.c_str(),
				"width", G_TYPE_INT, c_.iWidth,
				"height", G_TYPE_INT, c_.iHeight,
				NULL),
			NULL);
		
		g_object_set(G_OBJECT(appsrc),
			"stream-type", GST_APP_STREAM_TYPE_STREAM,
			"format", GST_FORMAT_TIME,
			"is-live", TRUE,
			"do-timestamp", TRUE,
			NULL);
			
		//! Here is where we added an idle loop before:
		hid = g_idle_add ((GSourceFunc) read_data, this);
		
		gstThread = std::thread(&GstAppPush::gstThreadFunc, this);
		open = true;
		
		return 0;		
	}
	
	void gstThreadFunc()
	{		
		gst_element_set_state(pipeline, GST_STATE_PLAYING);		
		
		g_main_loop_run(loop);
		
		printf("Stopping\n");
		
		gst_element_set_state(pipeline, GST_STATE_NULL);
		
		gst_object_unref(bus);		
		g_main_loop_unref(loop);
	}
	
	static gboolean bus_message(GstBus *bus, GstMessage *message, GstAppPush *app)
	{
		switch(GST_MESSAGE_TYPE(message)) 
		{
			case GST_MESSAGE_ERROR: {
				GError *err = NULL;
				gchar *dbg_info = NULL;
				
				gst_message_parse_error (message, &err, &dbg_info);
				g_printerr("Error from element %s: %s\n",
					GST_OBJECT_NAME (message->src), err->message);
				g_printerr("Debugging info: %s\n", (dbg_info) ? dbg_info: "none");
				g_error_free(err);
				g_free(dbg_info);
				g_main_loop_quit(app->loop);
				break;
			}
			case GST_MESSAGE_EOS:
				g_main_loop_quit(app->loop);
				break;
			default:
				break;
		}
		return TRUE;
	}
};
