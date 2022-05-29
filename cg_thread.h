static void* cg_thread_start(void *t);

struct cg_thread {
    unsigned int m_thread_id;
    benchmark_config* m_config;
    object_generator* m_obj_gen;
    client_group* m_cg;
    abstract_protocol* m_protocol;
    pthread_t m_thread;
    bool m_finished;

    cg_thread(unsigned int id, benchmark_config* config, object_generator* obj_gen) :
        m_thread_id(id), m_config(config), m_obj_gen(obj_gen), m_cg(NULL), m_protocol(NULL), m_finished(false)
    {
        m_protocol = protocol_factory(m_config->protocol);
        assert(m_protocol != NULL);

        m_cg = new client_group(m_config, m_protocol, m_obj_gen);
    }

    ~cg_thread()
    {
        if (m_cg != NULL) {
            delete m_cg;
        }
        if (m_protocol != NULL) {
            delete m_protocol;
        }
    }

    int prepare(void)
    {
        if (m_cg->create_clients(m_config->clients) < (int) m_config->clients)
            return -1;
        return m_cg->prepare();
    }

    int start(void)
    {
        return pthread_create(&m_thread, NULL, cg_thread_start, (void *)this);
    }

    void join(void)
    {
        int* retval;
        int ret;

        ret = pthread_join(m_thread, (void **)&retval);
        // std::this_thread::sleep_for(std::chrono::milliseconds(1000 * 3600));
        assert(ret == 0);
    }

};

static void* cg_thread_start(void *t)
{
    cg_thread* thread = (cg_thread*) t;
    thread->m_cg->run();
    thread->m_finished = true;

    return t;
}
