int lag2_initialize_node() {
    if (subl_graph == nullptr) {
        subl_graph = new SublGraph();
        subl_graph->n_nodes = 0;
        subl_graph->n_edges = 0;
        subl_graph->fixed = create_fixed(prob);
        subl_graph->node2 = (void *) xmalloc((prob->T + 2)*sizeof(_node2m_t *));
    }

  if(prob->graph->bedge == NULL) {
    prob->graph->n_nodes = prob->graph->n_edges = 0;
    prob->graph->bnode = create_benv(sizeof(_node2m_t));
    prob->graph->bedge = create_benv(sizeof(_edge2m_t));
    prob->graph->fixed = create_fixed(prob);
    prob->graph->node2 = (void *) xmalloc((prob->T + 2)*sizeof(_node2m_t *));
    ret = _lag2_initialize_node_forward(prob, u);
    ptable_free_adj(prob->graph->ptable);

    if(ret == SIPS_NORMAL) {
      ret = _lag2_check_time_window(prob);
    }
  } else {
    ret = SIPS_NORMAL;
  }

  return(ret);

}