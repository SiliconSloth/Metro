#pragma once

namespace git {
    typedef git_signature Signature;
    typedef git_tree * Tree;
    typedef git_oid OID;
    typedef git_object Object;
    typedef git_status_list StatusList;
    typedef git_commit Commit;
    typedef git_reset_t ResetType;
    typedef git_checkout_options CheckoutOptions;
    typedef git_status_options StatusOptions;
    typedef git_strarray StrArray;
    typedef git_index_matched_path_cb MatchedPathCallback;
    typedef class ConflictIterator ConflictIterator;
}