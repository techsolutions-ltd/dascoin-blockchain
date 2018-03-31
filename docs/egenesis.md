
Embedded genesis
----------------

Compile executable which references a specific genesis state.  The
full genesis state can be included by linking with `egensis_full`,
or only hashes (chain ID and hash of JSON) can be included by linking
with `egenesis_brief`.

The `GRAPHENE_EGENESIS_JSON` parameter specifies the `genesis.json`
to be included.  Note, you will have to delete your `cmake` leftovers
with

    make clean
    rm -f CMakeCache.txt
    find . -name CMakeFiles | xargs rm -Rf

The embedded data can be accessed by functions in `egenesis.hpp`.
