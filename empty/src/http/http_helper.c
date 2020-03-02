#include "http/http_helper.h"

/* The event group allows multiple bits for each event,
   but we only care about one event - are we connected
   to the AP with an IP? */

const int CONNECTED_BIT = BIT0;
/* Constants that aren't configurable in menuconfig */

static const char *TAG = "example";

static esp_err_t _http_event_handler(esp_http_client_event_t *evt)
{
    switch (evt->event_id)
    {
    case HTTP_EVENT_ERROR:
        ESP_LOGD(TAG, "HTTP_EVENT_ERROR");
        break;
    case HTTP_EVENT_ON_CONNECTED:
        ESP_LOGD(TAG, "HTTP_EVENT_ON_CONNECTED");
        break;
    case HTTP_EVENT_HEADER_SENT:
        ESP_LOGD(TAG, "HTTP_EVENT_HEADER_SENT");
        break;
    case HTTP_EVENT_ON_HEADER:
        ESP_LOGD(TAG, "HTTP_EVENT_ON_HEADER, key=%s, value=%s", evt->header_key, evt->header_value);
        break;
    case HTTP_EVENT_ON_DATA:
        ESP_LOGD(TAG, "HTTP_EVENT_ON_DATA, len=%d", evt->data_len);
        if (!esp_http_client_is_chunked_response(evt->client))
        {
            // Write out data
            // printf("%.*s", evt->data_len, (char*)evt->data);
        }
        break;
    case HTTP_EVENT_ON_FINISH:
        ESP_LOGD(TAG, "HTTP_EVENT_ON_FINISH");
        break;
    case HTTP_EVENT_DISCONNECTED:
        ESP_LOGD(TAG, "HTTP_EVENT_DISCONNECTED");
        break;
    }
    return ESP_OK;
}

static void http_get_task(void *pvParameters)
{
    phttp_param data = (phttp_param)pvParameters;
    if (!(data != NULL && data->url != NULL))
    {
        ESP_LOGI(TAG, "Invalid parament!(http_get_task)");
    }

    esp_http_client_config_t config = {
        .url = data->url,
        .event_handler = _http_event_handler,
    };

    esp_http_client_handle_t client = esp_http_client_init(&config);
    esp_err_t err = esp_http_client_perform(client);

    if (err == ESP_OK)
    {
        int content_length = esp_http_client_get_content_length(client);
        ESP_LOGI(TAG, "HTTP GET Status = %d, content_length = %d",
                 esp_http_client_get_status_code(client),
                 content_length);

        read_response(client, content_length);
    }
    else
    {
        ESP_LOGE(TAG, "HTTP GET request failed: %s", esp_err_to_name(err));
    }

    esp_http_client_cleanup(client);
}

static void read_response(esp_http_client_handle_t client, int const content_length)
{

    char buffer[4096];
    printf("Response: ");

    while (true)
    {
        int const read_len = esp_http_client_read(client, buffer, content_length);

        if (read_len == 0)
            break;

        if (read_len == -1)
        {
            printf("Error read!");
            return;
        }

        buffer[read_len - 1] = 0;
        printf(buffer);
        
    }
    printf("\nEnd");

}

static void http_post_task(phttp_param pvParameters)
{

    phttp_param data = (phttp_param)pvParameters;

    if (!(data != NULL && data->url != NULL && data->post_param != NULL))
    {
        ESP_LOGI(TAG, "Invalid parament!(http_get_task)");
    }

    esp_http_client_config_t config = {
        .url = data->url,
        .event_handler = _http_event_handler,
    };
    esp_http_client_handle_t client = esp_http_client_init(&config);

    esp_http_client_set_method(client, HTTP_METHOD_POST);
    esp_http_client_set_post_field(client, data->post_param, strlen(data->post_param) + 1);
    esp_err_t err = esp_http_client_perform(client);

    if (err == ESP_OK)
    {
        int content_length = esp_http_client_get_content_length(client);
        read_response(client, content_length);
    }
    else
    {
        ESP_LOGE(TAG, "HTTP POST request failed: %s", esp_err_to_name(err));
    }
}

static int get_post_handler(int argc, char **argv)
{

    http_param data;

    if (argc < 3)
    {
        printf("Invalid argument!");
        return 1;
    }

    if (strcmp(argv[1], "GET") == 0)
    {
        data.url = argv[2];
        http_get_task(&data);
    }
    else
    {
        //POST
    }

    return 1;
}

void register_http(void)
{

    const esp_console_cmd_t cmd = {
        .command = "http",
        .help = "http <REQ> <URL> <BODY>",
        .hint = NULL,
        .func = &get_post_handler,
    };

    ESP_ERROR_CHECK(esp_console_cmd_register(&cmd));
}