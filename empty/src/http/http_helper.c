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
    psetting_http data = (psetting_http)pvParameters;
    if (!(data != NULL && data->request != NULL))
    {
        ESP_LOGI(TAG, "Invalid parament!(http_get_task)");
    }

    esp_http_client_config_t config = {
        .url = data->request,
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

        char *buffer = malloc(MAX_HTTP_RECV_BUFFER);

        int read_len = esp_http_client_read(client, buffer, content_length);
        buffer[read_len] = 0;

        ESP_LOGI(TAG, "HTTP Stream reader Status = %d, content_length = %d, Buffer=%.*s",
                 esp_http_client_get_status_code(client),
                 read_len,
                 read_len,
                 buffer);

        data->output = buffer;
    }
    else
    {
        ESP_LOGE(TAG, "HTTP GET request failed: %s", esp_err_to_name(err));
    }

    esp_http_client_cleanup(client);
}

void send_http_get_request(psetting_http data)
{

    ESP_ERROR_CHECK(nvs_flash_init());
    xTaskCreate(&http_get_task, "http_get_task", 8192, data, 5, NULL);
}

static int get_post_handler(int argc, char **argv)
{

    setting_http data;

    if (argc < 3)
    {
        printf("Invalid argument!");
        return 1;
    }

    if (strcmp(argv[1], "GET") == 0)
    {

        data.request = argv[2];
        send_http_get_request(&data);

        if (data.output != NULL)
        {
            printf("Output http get:%s\r\n", data.output);	
            free(data.output);
            return 0;
        }

        printf("Error request!");

    }
    else{
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