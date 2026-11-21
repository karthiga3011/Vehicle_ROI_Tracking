
#include <gst/gst.h>
#include <stdio.h>

// Structure to store pipeline elements
typedef struct
{
    GstElement* pipeline;
    GstElement* source;
    GstElement* convert;
    GstElement* resample;
    GstElement* sink;

} CustomData;


// Callback function for newly created pads
static void pad_added_handler(
    GstElement* src,
    GstPad* new_pad,
    CustomData* data)
{
    GstPad* sink_pad =
        gst_element_get_static_pad(data->convert, "sink");

    // Check whether already linked
    if (gst_pad_is_linked(sink_pad))
    {
        g_print("Already linked!\n");
        gst_object_unref(sink_pad);
        return;
    }

    // Get the new pad information
    GstCaps* new_pad_caps =
        gst_pad_get_current_caps(new_pad);

    if (new_pad_caps == NULL)
    {
        g_print("Could not get pad capabilities.\n");
        gst_object_unref(sink_pad);
        return;
    }

    GstStructure* new_pad_struct =
        gst_caps_get_structure(new_pad_caps, 0);

    const gchar* new_pad_type =
        gst_structure_get_name(new_pad_struct);

    g_print("Detected stream: %s\n", new_pad_type);

    // Accept only audio streams
    if (g_str_has_prefix(new_pad_type, "audio/x-raw"))
    {
        GstPadLinkReturn ret =
            gst_pad_link(new_pad, sink_pad);

        if (ret == GST_PAD_LINK_OK)
        {
            g_print("Audio pad linked successfully!\n");
        }
        else
        {
            g_print("Audio pad linking failed!\n");
        }
    }
    else
    {
        g_print("Not an audio stream. Ignoring.\n");
    }

    gst_caps_unref(new_pad_caps);
    gst_object_unref(sink_pad);
}


int main(int argc, char* argv[])
{
    CustomData data;

    GstBus* bus;
    GstMessage* msg;

    // 1. Initialize GStreamer
    gst_init(&argc, &argv);

    // 2. Create elements
    data.source =
        gst_element_factory_make("uridecodebin", "source");

    data.convert =
        gst_element_factory_make("audioconvert", "convert");

    data.resample =
        gst_element_factory_make("audioresample", "resample");

    data.sink =
        gst_element_factory_make("autoaudiosink", "sink");

    // 3. Create pipeline
    data.pipeline =
        gst_pipeline_new("test-pipeline");

    if (!data.pipeline ||
        !data.source ||
        !data.convert ||
        !data.resample ||
        !data.sink)
    {
        g_printerr("Could not create GStreamer elements!\n");
        return -1;
    }

    // 4. Add elements to pipeline
    gst_bin_add_many(
        GST_BIN(data.pipeline),
        data.source,
        data.convert,
        data.resample,
        data.sink,
        NULL
    );

    // 5. Link audio elements
    if (!gst_element_link_many(
        data.convert,
        data.resample,
        data.sink,
        NULL))
    {
        g_printerr("Could not link audio elements!\n");
        gst_object_unref(data.pipeline);
        return -1;
    }

    // 6. Set media URL
    g_object_set(
        data.source,
        "uri",
        "https://gstreamer.freedesktop.org/data/media/sintel_trailer-480p.webm",
        NULL
    );

    // 7. Connect dynamic pad signal
    g_signal_connect(
        data.source,
        "pad-added",
        G_CALLBACK(pad_added_handler),
        &data
    );

    // 8. Start playback
    GstStateChangeReturn ret =
        gst_element_set_state(
            data.pipeline,
            GST_STATE_PLAYING
        );

    if (ret == GST_STATE_CHANGE_FAILURE)
    {
        g_printerr("Unable to start pipeline!\n");
        gst_object_unref(data.pipeline);
        return -1;
    }

    g_print("GStreamer Tutorial 3 started!\n");

    // 9. Listen for messages
    bus = gst_element_get_bus(data.pipeline);

    gboolean terminate = FALSE;

    while (!terminate)
    {
        msg = gst_bus_timed_pop_filtered(
            bus,
            GST_CLOCK_TIME_NONE,
            (GstMessageType)(
                GST_MESSAGE_ERROR |
                GST_MESSAGE_EOS |
                GST_MESSAGE_STATE_CHANGED
            )
        );

        if (msg != NULL)
        {
            switch (GST_MESSAGE_TYPE(msg))
            {
            case GST_MESSAGE_ERROR:
            {
                GError* err = NULL;
                gchar* debug_info = NULL;

                gst_message_parse_error(
                    msg, &err, &debug_info);

                g_printerr("Error: %s\n", err->message);

                g_clear_error(&err);
                g_free(debug_info);

                terminate = TRUE;
                break;
            }

            case GST_MESSAGE_EOS:
                g_print("Playback completed!\n");
                terminate = TRUE;
                break;

            case GST_MESSAGE_STATE_CHANGED:

                if (GST_MESSAGE_SRC(msg) ==
                    GST_OBJECT(data.pipeline))
                {
                    GstState old_state;
                    GstState new_state;
                    GstState pending_state;

                    gst_message_parse_state_changed(
                        msg,
                        &old_state,
                        &new_state,
                        &pending_state
                    );

                    g_print(
                        "Pipeline: %s -> %s\n",
                        gst_element_state_get_name(old_state),
                        gst_element_state_get_name(new_state)
                    );
                }
                break;

            default:
                break;
            }

            gst_message_unref(msg);
        }
    }

    // 10. Cleanup
    gst_element_set_state(
        data.pipeline, GST_STATE_NULL);

    gst_object_unref(bus);
    gst_object_unref(data.pipeline);

    g_print("Tutorial 3 completed!\n");

    return 0;
}
