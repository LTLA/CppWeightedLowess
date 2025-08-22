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
      <type>Data_ *</type>
      <name>weights</name>
      <anchorfile>structWeightedLowess_1_1Options.html</anchorfile>
      <anchor>a3c3cae26a248351a3fd78bbfd8f15b3d</anchor>
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
      <anchor>aea5069601b1463a43524e0035255349f</anchor>
      <arglist>(const std::size_t num_points, const Sortable_ *x)</arglist>
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
      <anchor>abd8112d8539bfaee9fe03646cbeaf465</anchor>
      <arglist>(const std::size_t num_points, const Sortable_ *x)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>permute</name>
      <anchorfile>classWeightedLowess_1_1SortBy.html</anchorfile>
      <anchor>ac3af7bb0c3a06421b855a59c7213ff48</anchor>
      <arglist>(Data_ *data, Used_ *work) const</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>permute</name>
      <anchorfile>classWeightedLowess_1_1SortBy.html</anchorfile>
      <anchor>a38b636960bd52fb45be7b476974baeab</anchor>
      <arglist>(std::initializer_list&lt; Data_ &gt; data, Used_ *work) const</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>permute</name>
      <anchorfile>classWeightedLowess_1_1SortBy.html</anchorfile>
      <anchor>ac36619bc878340236831305133040486</anchor>
      <arglist>(DataPointers_ data, Used_ *work) const</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>permute</name>
      <anchorfile>classWeightedLowess_1_1SortBy.html</anchorfile>
      <anchor>a5238cd266b2380334778c2f09beb2d1f</anchor>
      <arglist>(Data_ *data, std::vector&lt; Used_ &gt; &amp;work) const</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>permute</name>
      <anchorfile>classWeightedLowess_1_1SortBy.html</anchorfile>
      <anchor>a19fcf75ba3a6117474439dcbbd2d9b33</anchor>
      <arglist>(std::initializer_list&lt; Data_ &gt; data, std::vector&lt; Used_ &gt; &amp;work) const</arglist>
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
      <anchor>a0e21a76c6f8a1a6f43129efd3a4fcc1d</anchor>
      <arglist>(Data_ *data, Used_ *work) const</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>unpermute</name>
      <anchorfile>classWeightedLowess_1_1SortBy.html</anchorfile>
      <anchor>a4d316ac11c79c3a80672067756362384</anchor>
      <arglist>(std::initializer_list&lt; Data_ * &gt; data, Used_ *work) const</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>unpermute</name>
      <anchorfile>classWeightedLowess_1_1SortBy.html</anchorfile>
      <anchor>a15a41028fc0a28a7f6c97878b790d6b6</anchor>
      <arglist>(DataPointers_ data, Used_ *work) const</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>unpermute</name>
      <anchorfile>classWeightedLowess_1_1SortBy.html</anchorfile>
      <anchor>a0127fde2589669bff5f2b796c67e61dc</anchor>
      <arglist>(Data_ *data, std::vector&lt; Used_ &gt; &amp;work) const</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>unpermute</name>
      <anchorfile>classWeightedLowess_1_1SortBy.html</anchorfile>
      <anchor>a129e7a5205ba62ffd301499333ea1c6d</anchor>
      <arglist>(std::initializer_list&lt; Data_ * &gt; data, std::vector&lt; Used_ &gt; &amp;work) const</arglist>
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
    <class kind="struct">WeightedLowess::Options</class>
    <class kind="struct">WeightedLowess::PrecomputedWindows</class>
    <class kind="struct">WeightedLowess::Results</class>
    <class kind="class">WeightedLowess::SortBy</class>
    <member kind="function">
      <type>void</type>
      <name>compute</name>
      <anchorfile>namespaceWeightedLowess.html</anchorfile>
      <anchor>ad11983a78a81e353c458a203d4f581c9</anchor>
      <arglist>(const std::size_t num_points, const Data_ *x, const PrecomputedWindows&lt; Data_ &gt; &amp;windows, const Data_ *y, Data_ *fitted, Data_ *robust_weights, const Options&lt; Data_ &gt; &amp;opt)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>compute</name>
      <anchorfile>namespaceWeightedLowess.html</anchorfile>
      <anchor>a4aaf79d3cc00d2e77b6c3ac887c98a22</anchor>
      <arglist>(const std::size_t num_points, const Data_ *x, const Data_ *y, Data_ *fitted, Data_ *robust_weights, const Options&lt; Data_ &gt; &amp;opt)</arglist>
    </member>
    <member kind="function">
      <type>Results&lt; Data_ &gt;</type>
      <name>compute</name>
      <anchorfile>namespaceWeightedLowess.html</anchorfile>
      <anchor>ae05ea1089c2a797e1c5cf2e7115c928d</anchor>
      <arglist>(const std::size_t num_points, const Data_ *x, const Data_ *y, const Options&lt; Data_ &gt; &amp;opt)</arglist>
    </member>
    <member kind="function">
      <type>void</type>
      <name>parallelize</name>
      <anchorfile>namespaceWeightedLowess.html</anchorfile>
      <anchor>adfb0d596184fac8c548a45cb6fb8fbb1</anchor>
      <arglist>(const int num_workers, const Task_ num_tasks, Run_ run_task_range)</arglist>
    </member>
    <member kind="function">
      <type>PrecomputedWindows&lt; Data_ &gt;</type>
      <name>define_windows</name>
      <anchorfile>namespaceWeightedLowess.html</anchorfile>
      <anchor>a30dc5c465fd1c49a9b8a0d8e1a51e1ee</anchor>
      <arglist>(const std::size_t num_points, const Data_ *x, const Options&lt; Data_ &gt; &amp;opt)</arglist>
    </member>
  </compound>
  <compound kind="page">
    <name>index</name>
    <title>Weighted LOWESS for C++</title>
    <filename>index.html</filename>
    <docanchor file="index.html">md__2github_2workspace_2README</docanchor>
  </compound>
</tagfile>
