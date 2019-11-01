#pragma once

namespace git {
    typedef git_signature Signature;
    typedef git_reset_t ResetType;
    typedef git_checkout_options CheckoutOptions;
    typedef git_index_matched_path_cb MatchedPathCallback;
    typedef git_remote_callbacks Callbacks;
    typedef class ConflictIterator ConflictIterator;
}