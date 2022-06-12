
static std::string last_interval_json_stats = "{}";
static std::mutex stats_mutex;

std::string double_to_str(const double value) {
    std::stringstream stream;
    stream << std::fixed << std::setprecision(2) << value;
    std::string value_str = stream.str();
    return value_str;
}

class StatsWebServer {
    std::promise<void> signal_exit; //A promise object
    std::thread serve_http_thread;
    unsigned short api_port;
    unsigned short api_report_interval;

    static void serve_request(struct mg_connection *c, int ev, void *ev_data, void *fn_data) {
        if (ev == MG_EV_HTTP_MSG) {
          struct mg_http_message *hm = (struct mg_http_message *) ev_data;
          if (mg_http_match_uri(hm, "/api/lastminstats")) {
            const char* stats = last_interval_json_stats.c_str();
            fprintf(stderr, "Stats API Response: %s\n", stats);
            mg_http_reply(c, 200, "Content-Type: application/json\r\n", "%s", stats);  // Serve dynamic content
          } else {
            struct mg_http_serve_opts opts = {.root_dir = "."};   // Serve
            mg_http_serve_dir(c, hm, &opts);                 // static content
          }
        }
    }

    static void serve_http(unsigned short port, std::future<void> future)
    {
        struct mg_mgr mgr;
        char port_str[25];
        snprintf(port_str, sizeof(port_str)-1, "http://0.0.0.0:%u", port);
        mg_mgr_init(&mgr);                                      // Init manager
        mg_http_listen(&mgr, port_str, serve_request, &mgr);  // Setup listener
        fprintf(stderr, "Stats Server: Started listening on port %u.\n", port);
        while (future.wait_for(std::chrono::milliseconds(1)) == std::future_status::timeout){
          mg_mgr_poll(&mgr, 1000);
        }
        mg_mgr_free(&mgr);                                      // Cleanup
    }

    std::string get_last_m_sec_stats_json(one_sec_cmd_stats cmd_stats, std::vector<float> quantile_list){
        std::string json_str = "";
        json_str+="{";
        unsiged short interval = cfg->api_report_interval;
        const bool has_samples = hdr_total_count(cmd_stats.latency_histogram)>0;
        const bool sec_has_samples = hdr_total_count(cmd_stats.latency_histogram)>0;
        const double sec_avg_latency = sec_has_samples ? hdr_mean(cmd_stats.latency_histogram)/ (double) LATENCY_HDR_RESULTS_MULTIPLIER : 0.0;
        const double sec_min_latency = has_samples ? hdr_min(cmd_stats.latency_histogram)/ (double) LATENCY_HDR_RESULTS_MULTIPLIER : 0.0;
        const double sec_max_latency = has_samples ? hdr_max(cmd_stats.latency_histogram)/ (double) LATENCY_HDR_RESULTS_MULTIPLIER : 0.0;
        json_str += "\"Count\":" + std::to_string(hdr_total_count(cmd_stats.latency_histogram)) + ",";
        json_str += "\"OpsPerSecond\":" + std::to_string(cmd_stats.m_ops/interval) + ",";
        json_str += "\"AverageLatency\":" + double_to_str(sec_avg_latency) + ",";
        json_str += "\"MinLatency\":" + double_to_str(sec_min_latency) + ",";
        json_str += "\"MaxLatency\":" + double_to_str(sec_max_latency) + ",";
        json_str += "\"LatencyPercentiles\":{";
        for (std::size_t i = 0; i < quantile_list.size(); i++){
            const float quantile = quantile_list[i];
            char quantile_header[8];
            snprintf(quantile_header,sizeof(quantile_header)-1,"p%.2f", quantile);
            const double value = hdr_value_at_percentile(cmd_stats.latency_histogram, quantile )/ (double) LATENCY_HDR_RESULTS_MULTIPLIER;
            json_str += "\""+std::string(quantile_header)+"\":"+ double_to_str(value);
            if(i < quantile_list.size() - 1) {
                json_str += ",";
            }
        }
        json_str += "}}";
        return json_str;
    }

    public:
    StatsWebServer(unsigned short api_port, unsigned short api_report_interval) {
      this->api_port = api_port;
      this->api_report_interval = api_report_interval;
    }

    void start_server() {
        std::future<void> future = signal_exit.get_future();//create future objects
        serve_http_thread = std::thread(&StatsWebServer::serve_http, this->api_port, std::move(future));
    }

    void stop_server(){
        signal_exit.set_value(); //set value into promise
        serve_http_thread.join();
    }

    void calc_last_interval_stats(std::vector<cg_thread*> threads, std::vector<float> quantile_list) {
        one_second_stats stats(0);
        for (std::vector<cg_thread*>::iterator i = threads.begin(); i != threads.end(); i++) {
            stats.merge((*i)->m_cg->get_interval_stats());
        }
        std::string json_str = "";
        json_str += "{\"GetStats\":";
        json_str += get_last_m_sec_stats_json(stats.m_set_cmd, quantile_list);
        json_str += ",\"SetStats\":";
        json_str += get_last_m_sec_stats_json(stats.m_get_cmd, quantile_list);
        json_str += "}";
        stats_mutex.lock();
        last_interval_json_stats = json_str;
        stats_mutex.unlock();
    }
};
