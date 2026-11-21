
#include <gst/gst.h>
#include <stdio.h>

int main(int argc, char* argv[])
{
    GstElement* pipeline;
    GstElement* source;
    GstElement* sink;

    GstBus* bus;
    GstMessage* msg;

    // 1. Initialize GStreamer
    gst_init(&argc, &argv);

    // 2. Create elements
    source = gst_element_factory_make(
        "videotestsrc", "source");

    sink = gst_element_factory_make(
        "autovideosink", "sink");

    // 3. Create pipeline
    pipeline = gst_pipeline_new("test-pipeline");

    if (!pipeline || !source || !sink)
    {
        g_printerr("Element creation failed!\n");
        return -1;
    }

    // 4. Add elements
    gst_bin_add_many(
        GST_BIN(pipeline),
        source,
        sink,
        NULL
    );

    // 5. Link elements
    if (!gst_element_link(source, sink))
    {
        g_printerr("Linking failed!\n");
        return -1;
    }

    // 6. Start pipeline
    gst_element_set_state(
        pipeline, GST_STATE_PLAYING);

    g_print("Video pipeline started!\n");

    // 7. Wait for messages
    bus = gst_element_get_bus(pipeline);

    msg = gst_bus_timed_pop_filtered(
        bus,
        GST_CLOCK_TIME_NONE,
        (GstMessageType)(
            GST_MESSAGE_ERROR |
            GST_MESSAGE_EOS
        )
    );

    if (msg)
    {
        if (GST_MESSAGE_TYPE(msg) == GST_MESSAGE_ERROR)
        {
            GError* err = NULL;
            gchar* debug_info = NULL;

            gst_message_parse_error(
                msg, &err, &debug_info);

            g_printerr("Error: %s\n", err->message);

            g_clear_error(&err);
            g_free(debug_info);
        }

        gst_message_unref(msg);
    }

    // 8. Cleanup
    gst_element_set_state(
        pipeline, GST_STATE_NULL);

    gst_object_unref(bus);
    gst_object_unref(pipeline);

    return 0;
}
