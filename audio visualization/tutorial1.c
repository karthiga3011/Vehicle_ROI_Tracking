
#include <gst/gst.h>
#include <stdio.h>

int main(int argc, char *argv[])
{
    GstElement *pipeline;
    GstBus *bus;
    GstMessage *msg;

    // 1. Initialize GStreamer
    gst_init(&argc, &argv);

    // 2. Create a playback pipeline
    pipeline = gst_element_factory_make("playbin", "player");

    if (!pipeline)
    {
        g_printerr("Could not create playbin element.\n");
        return -1;
    }

    // 3. Set the media file or URL
    g_object_set(
        pipeline,
        "uri",
        "https://gstreamer.freedesktop.org/data/media/sintel_trailer-480p.webm",
        NULL
    );

    // 4. Start playback
    gst_element_set_state(pipeline, GST_STATE_PLAYING);

    g_print("Playing media...\n");

    // 5. Get pipeline messages
    bus = gst_element_get_bus(pipeline);

    msg = gst_bus_timed_pop_filtered(
        bus,
        GST_CLOCK_TIME_NONE,
        GST_MESSAGE_ERROR | GST_MESSAGE_EOS
    );

    // 6. Handle messages
    if (msg != NULL)
    {
        switch (GST_MESSAGE_TYPE(msg))
        {
        case GST_MESSAGE_ERROR:
        {
            GError *err;
            gchar *debug_info;

            gst_message_parse_error(msg, &err, &debug_info);

            g_printerr("Error: %s\n", err->message);
            g_printerr("Debug information: %s\n",
                       debug_info ? debug_info : "none");

            g_clear_error(&err);
            g_free(debug_info);
            break;
        }

        case GST_MESSAGE_EOS:
            g_print("Playback completed successfully!\n");
            break;

        default:
            break;
        }

        gst_message_unref(msg);
    }

    // 7. Stop and release resources
    gst_element_set_state(pipeline, GST_STATE_NULL);

    gst_object_unref(bus);
    gst_object_unref(pipeline);

    g_print("GStreamer application closed.\n");

    return 0;
}
