#pragma once

namespace git {
    typedef git_signature Signature;
    typedef git_reset_t ResetType;
    typedef git_checkout_options CheckoutOptions;
    typedef git_index_matched_path_cb MatchedPathCallback;
    typedef git_remote_callbacks Callbacks;
    typedef git_fetch_options FetchOps;
    typedef git_status_options StatusOptions;
    typedef git_merge_analysis_t MergeAnalysis;
    typedef git_push_options PushOptions;
}