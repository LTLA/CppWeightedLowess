<?xml version='1.0' encoding='UTF-8' standalone='yes' ?>
<tagfile doxygen_version="1.12.0">
  <compound kind="file">
    <name>compute.hpp</name>
    <path>WeightedLowess/</path>
    <filename>compute_8hpp.html</filename>
    <includes id="Options_8hpp" name="Options.hpp" local="yes" import="no" module="no" objc="no">Options.hpp</includes>
    <class kind="struct">WeightedLowess::Results</class>
    <namespace>WeightedLowess</namespace>
  </compound>
  <compound kind="file">
    <name>interpolate.hpp</name>
    <path>WeightedLowess/</path>
    <filename>interpolate_8hpp.html</filename>
    <includes id="window_8hpp" name="window.hpp" local="yes" import="no" module="no" objc="no">window.hpp</includes>
    <class kind="struct">WeightedLowess::AssignedSegments</class>
    <namespace>WeightedLowess</namespace>
  </compound>
  <compound kind="file">
    <name>Options.hpp</name>
    <path>WeightedLowess/</path>
    <filename>Options_8hpp.html</filename>
    <class kind="struct">WeightedLowess::Options</class>
    <namespace>WeightedLowess</namespace>
  </compound>
  <compound kind="file">
    <name>parallelize.hpp</name>
    <path>WeightedLowess/</path>
    <filename>parallelize_8hpp.html</filename>
    <namespace>WeightedLowess</namespace>
  </compound>
  <compound kind="file">
    <name>SortBy.hpp</name>
    <path>WeightedLowess/</path>
    <filename>SortBy_8hpp.html</filename>
    <class kind="class">WeightedLowess::SortBy</class>
    <namespace>WeightedLowess</namespace>
  </compound>
  <compound kind="file">
    <name>WeightedLowess.hpp</name>
    <path>WeightedLowess/</path>
    <filename>WeightedLowess_8hpp.html</filename>
    <includes id="compute_8hpp" name="compute.hpp" local="yes" import="no" module="no" objc="no">compute.hpp</includes>
    <includes id="interpolate_8hpp" name="interpolate.hpp" local="yes" import="no" module="no" objc="no">interpolate.hpp</includes>
    <includes id="SortBy_8hpp" name="SortBy.hpp" local="yes" import="no" module="no" objc="no">SortBy.hpp</includes>
    <includes id="Options_8hpp" name="Options.hpp" local="yes" import="no" module="no" objc="no">Options.hpp</includes>
    <namespace>WeightedLowess</namespace>
  </compound>
  <compound kind="file">
    <name>window.hpp</name>
    <path>WeightedLowess/</path>
    <filename>window_8hpp.html</filename>
    <includes id="Options_8hpp" name="Options.hpp" local="yes" import="no" module="no" objc="no">Options.hpp</includes>
    <includes id="parallelize_8hpp" name="parallelize.hpp" local="yes" import="no" module="no" objc="no">parallelize.hpp</includes>
    <class kind="struct">WeightedLowess::PrecomputedWindows</class>
    <namespace>WeightedLowess</namespace>
  </compound>
  <compound kind="struct">
    <name>WeightedLowess::AssignedSegments</name>
    <filename>structWeightedLowess_1_1AssignedSegments.html</filename>
  </compound>
  <compound kind="struct">
    <name>WeightedLowess::Options</name>
    <filename>structWeightedLowess_1_1Options.html</filename>
    <templarg>typename Data_</templarg>
    <member kind="variable">
      <type>Data_</type>
      <name>span</name>
      <anchorfile>structWeightedLowess_1_1Options.html</anchorfile>
      <anchor>a820614e4b0971b12560f420025876549</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>bool</type>
      <name>span_as_proportion</name>
      <anchorfile>structWeightedLowess_1_1Options.html</anchorfile>
      <anchor>ab6408e30d42c2d6f328b1bc002c8e2d9</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>Data_</type>
      <name>minimum_width</name>
      <anchorfile>structWeightedLowess_1_1Options.html</anchorfile>
      <anchor>a896253316d390568bbadd1ce40fe4ee9</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>std::size_t</type>
      <name>anchors</name>
      <anchorfile>structWeightedLowess_1_1Options.html</anchorfile>
      <anchor>a5e34709595bc216d41f01fc60ca2b7fb</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>int</type>
      <name>iterations</name>
      <anchorfile>structWeightedLowess_1_1Options.html</anchorfile>
      <anchor>ae1c47f860a4d16df2a54a6048b49f96b</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>Data_</type>
      <name>delta</name>
      <anchorfile>structWeightedLowess_1_1Options.html</anchorfile>
      <anchor>a55bc0add48ea9e083cc527773a182192</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>const Data_ *</type>
      <name>weights</name>
      <anchorfile>structWeightedLowess_1_1Options.html</anchorfile>
      <anchor>a098463ee6beb3d9207725b0d597c42b2</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>bool</type>
      <name>frequency_weights</name>
      <anchorfile>structWeightedLowess_1_1Options.html</anchorfile>
      <anchor>a414dbeb65809f19b12ce7877c2c048be</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>int</type>
      <name>num_threads</name>
      <anchorfile>structWeightedLowess_1_1Options.html</anchorfile>
      <anchor>afe399f94ea4c77261a2583bfe1aa4c2f</anchor>
      <arglist></arglist>
    </member>
  </compound>
  <compound kind="struct">
    <name>WeightedLowess::PrecomputedWindows</name>
    <filename>structWeightedLowess_1_1PrecomputedWindows.html</filename>
    <templarg>typename Data_</templarg>
  </compound>
  <compound kind="struct">
    <name>WeightedLowess::Results</name>
    <filename>structWeightedLowess_1_1Results.html</filename>
    <templarg>typename Data_</templarg>
    <member kind="function">
      <type></type>
      <name>Results</name>
      <anchorfile>structWeightedLowess_1_1Results.html</anchorfile>
      <anchor>afb46837329d3b550dc40114bb283c508</anchor>
      <arglist>(const std::size_t n)</arglist>
    </member>
    <member kind="variable">
      <type>std::vector&lt; Data_ &gt;</type>
      <name>fitted</name>
      <anchorfile>structWeightedLowess_1_1Results.html</anchorfile>
      <anchor>a8314067146c939368ae593e09d9dca27</anchor>
      <arglist></arglist>
    </member>
    <member kind="variable">
      <type>std::vector&lt; Data_ &gt;</type>
      <name>robust_weights</name>
      <anchorfile>structWeightedLowess_1_1Results.html</anchorfile>
      <anchor>aff5b0f38c864f6e8e7323118e4c19ce9</anchor>
      <arglist></arglist>
    </member>
  </compound>
  <compound kind="class">
    <name>WeightedLowess::SortBy</name>
    <filename>classWeightedLowess_1_1SortBy.html</filename>
    <member kind="function">
      <type></type>
      <name>SortBy</name>
      <anchorfile>classWeightedLowess_1_1SortBy.html</anchorfile>
      <anchor>af2cee7cf6dbdc90b92bd73083f0430e0</anchor>
      <arglist>(const std::size_t num_points, const Sortable_ *const x)</arglist>
    </member>
    <member kind="function">
      <type></type>
      <name>SortBy</name>
      <anchorfile>classWeightedLowess_1_1SortBy.html</anchorfile>
      <anchor>ab86761ca01148dd783d9352271aa4d89</anchor>
      <arglist>()=default</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>set</name>
      <anchorfile>classWeightedLowess_1_1SortBy.html</anchorfile>
      <anchor>a01f860cdf7398e507ed571ed2fe360f3</anchor>
      <arglist>(const std::size_t num_points, const Sortable_ *const x)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>permute</name>
      <anchorfile>classWeightedLowess_1_1SortBy.html</anchorfile>
      <anchor>ab7b17e28ae012052e63814a34a6d84ff</anchor>
      <arglist>(Data_ *const data, Used_ *const work) const</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>permute</name>
      <anchorfile>classWeightedLowess_1_1SortBy.html</anchorfile>
      <anchor>a24a20aef379102e955469a69243cdb72</anchor>
      <arglist>(std::initializer_list&lt; Data_ *const &gt; data, Used_ *const work) const</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>permute</name>
      <anchorfile>classWeightedLowess_1_1SortBy.html</anchorfile>
      <anchor>a359cefdd526033ede6f559f549b0068c</anchor>
      <arglist>(DataPointers_ data, Used_ *const work) const</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>permute</name>
      <anchorfile>classWeightedLowess_1_1SortBy.html</anchorfile>
      <anchor>a6fb0855317b92409f8e67b732aafb470</anchor>
      <arglist>(Data_ *const data, std::vector&lt; Used_ &gt; &amp;work) const</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>permute</name>
      <anchorfile>classWeightedLowess_1_1SortBy.html</anchorfile>
      <anchor>a40b99986dbc31582edff6e23c5673077</anchor>
      <arglist>(std::initializer_list&lt; Data_ *const &gt; data, std::vector&lt; Used_ &gt; &amp;work) const</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>permute</name>
      <anchorfile>classWeightedLowess_1_1SortBy.html</anchorfile>
      <anchor>a8f4e951ac12f1293037f1ba363f070a6</anchor>
      <arglist>(DataPointers_ data, std::vector&lt; Used_ &gt; &amp;work) const</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>unpermute</name>
      <anchorfile>classWeightedLowess_1_1SortBy.html</anchorfile>
      <anchor>a3e2956fd6992ff180a084b90cf83d0cd</anchor>
      <arglist>(Data_ *const data, Used_ *const work) const</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>unpermute</name>
      <anchorfile>classWeightedLowess_1_1SortBy.html</anchorfile>
      <anchor>a3d9cc05b52a3fd3fdd2eaf91676f6f39</anchor>
      <arglist>(std::initializer_list&lt; Data_ *const &gt; data, Used_ *const work) const</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>unpermute</name>
      <anchorfile>classWeightedLowess_1_1SortBy.html</anchorfile>
      <anchor>a4aac2ee6b62693e4882e331de8b68610</anchor>
      <arglist>(DataPointers_ data, Used_ *const work) const</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>unpermute</name>
      <anchorfile>classWeightedLowess_1_1SortBy.html</anchorfile>
      <anchor>af988b9ce8382881f855777204da54e64</anchor>
      <arglist>(Data_ *const data, std::vector&lt; Used_ &gt; &amp;work) const</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>unpermute</name>
      <anchorfile>classWeightedLowess_1_1SortBy.html</anchorfile>
      <anchor>a83c038bdd56fd5d84eee2a9123835bf8</anchor>
      <arglist>(std::initializer_list&lt; Data_ *const &gt; data, std::vector&lt; Used_ &gt; &amp;work) const</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>unpermute</name>
      <anchorfile>classWeightedLowess_1_1SortBy.html</anchorfile>
      <anchor>ad25d03e0aa1de5d19594c6d3ac52f215</anchor>
      <arglist>(DataPointers_ data, std::vector&lt; Used_ &gt; &amp;work) const</arglist>
    </member>
  </compound>
  <compound kind="namespace">
    <name>WeightedLowess</name>
    <filename>namespaceWeightedLowess.html</filename>
    <class kind="struct">WeightedLowess::AssignedSegments</class>
    <class kind="struct">WeightedLowess::Options</class>
    <class kind="struct">WeightedLowess::PrecomputedWindows</class>
    <class kind="struct">WeightedLowess::Results</class>
    <class kind="class">WeightedLowess::SortBy</class>
    <member kind="function">
      <type>void</type>
      <name>compute</name>
      <anchorfile>namespaceWeightedLowess.html</anchorfile>
      <anchor>a86b5b598907a786c4120963b67058864</anchor>
      <arglist>(const std::size_t num_points, const Data_ *const x, const PrecomputedWindows&lt; Data_ &gt; &amp;windows, const Data_ *const y, Data_ *const fitted, Data_ *robust_weights, const Options&lt; Data_ &gt; &amp;opt)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>compute</name>
      <anchorfile>namespaceWeightedLowess.html</anchorfile>
      <anchor>a2e434916498f95c65fe6f175e4d4ec07</anchor>
      <arglist>(const std::size_t num_points, const Data_ *const x, const Data_ *const y, Data_ *const fitted, Data_ *const robust_weights, const Options&lt; Data_ &gt; &amp;opt)</arglist>
    </member>
    <member kind="function">
      <type>Results&lt; Data_ &gt;</type>
      <name>compute</name>
      <anchorfile>namespaceWeightedLowess.html</anchorfile>
      <anchor>a75f25c7e955ed7731becb59c13c6fa90</anchor>
      <arglist>(const std::size_t num_points, const Data_ *const x, const Data_ *const y, const Options&lt; Data_ &gt; &amp;opt)</arglist>
    </member>
    <member kind="function">
      <type>AssignedSegments</type>
      <name>assign_to_segments</name>
      <anchorfile>namespaceWeightedLowess.html</anchorfile>
      <anchor>a63e5d05d4cbdd95dc99bd7a0907947c4</anchor>
      <arglist>(const Data_ *const x_fit, const PrecomputedWindows&lt; Data_ &gt; &amp;windows_fit, const std::size_t num_points_out, const Data_ *const x_out)</arglist>
    </member>
    <member kind="function">
      <type>std::pair&lt; std::size_t, std::size_t &gt;</type>
      <name>get_interpolation_boundaries</name>
      <anchorfile>namespaceWeightedLowess.html</anchorfile>
      <anchor>ad53f508204319f853c558dd2b157f909</anchor>
      <arglist>(const AssignedSegments &amp;assigned_out)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>interpolate</name>
      <anchorfile>namespaceWeightedLowess.html</anchorfile>
      <anchor>a6eaa8c82fa05a7f3ea5b50fb43af185a</anchor>
      <arglist>(const Data_ *const x_fit, const PrecomputedWindows&lt; Data_ &gt; &amp;windows_fit, const Data_ *const fitted_fit, const Data_ *const x_out, const AssignedSegments &amp;assigned_out, Data_ *const fitted_out, int num_threads)</arglist>
    </member>
    <member kind="function">
      <type>std::pair&lt; std::size_t, std::size_t &gt;</type>
      <name>interpolate</name>
      <anchorfile>namespaceWeightedLowess.html</anchorfile>
      <anchor>a42db34b74c2f149026e1d7aea443e056</anchor>
      <arglist>(const Data_ *const x_fit, const PrecomputedWindows&lt; Data_ &gt; &amp;windows_fit, const Data_ *const fitted_fit, const std::size_t num_points_out, const Data_ *const x_out, Data_ *const fitted_out, int num_threads)</arglist>
    </member>
    <member kind="function">
      <type>int</type>
      <name>parallelize</name>
      <anchorfile>namespaceWeightedLowess.html</anchorfile>
      <anchor>abf997aef63f77e296cb7be1931fbe6e1</anchor>
      <arglist>(const int num_workers, const Task_ num_tasks, Run_ run_task_range)</arglist>
    </member>
    <member kind="function">
      <type>PrecomputedWindows&lt; Data_ &gt;</type>
      <name>define_windows</name>
      <anchorfile>namespaceWeightedLowess.html</anchorfile>
      <anchor>a42bddecb5f16e2cc5586c8c956aeda38</anchor>
      <arglist>(const std::size_t num_points, const Data_ *const x, const Options&lt; Data_ &gt; &amp;opt)</arglist>
    </member>
  </compound>
  <compound kind="page">
    <name>index</name>
    <title>Weighted LOWESS for C++</title>
    <filename>index.html</filename>
    <docanchor file="index.html">md__2github_2workspace_2README</docanchor>
  </compound>
</tagfile>
