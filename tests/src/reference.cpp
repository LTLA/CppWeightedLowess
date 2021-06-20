#include <gtest/gtest.h>
#include "WeightedLowess/WeightedLowess.hpp"
#include "utils.h"

/* Using the limma::weightedLowess implementation as a reference:
 *
 * set.seed(10)
 * x <- runif(100)
 * cat(paste(strwrap(paste(x, collapse=", "), 150), collapse="\n"))
 *
 * y <- rnorm(100)
 * cat(paste(strwrap(paste(y, collapse=", "), 150), collapse="\n"))
 *
 * output <- limma::weightedLowess(x, y)
 * cat(paste(strwrap(paste(output$fitted, collapse=", "), 150), collapse="\n"))
 * cat(paste(strwrap(paste(output$weights, collapse=", "), 150), collapse="\n"))
 *
 * # Check that interpolation is done consistently.
 * output <- limma::weightedLowess(x, y, npts=10)
 * cat(paste(strwrap(paste(output$fitted, collapse=", "), 150), collapse="\n"))
 *
 * # Check same behavior with an odd number of points.
 * output <- limma::weightedLowess(head(x, -1), head(y, -1)) 
 * cat(paste(strwrap(paste(output$fitted, collapse=", "), 150), collapse="\n"))
 * 
 */

std::vector<double> test_x {
    0.507478203158826, 0.306768506066874, 0.426907666493207, 0.693102080840617, 0.0851359688676894, 0.225436616456136, 0.274530522990972,
    0.272305066231638, 0.615829307818785, 0.429671525489539, 0.651655666995794, 0.567737752571702, 0.113508982118219, 0.595925305271521,
    0.358049975009635, 0.428809418343008, 0.0519033221062273, 0.264177667442709, 0.398790730861947, 0.836134143406525, 0.864721225807443,
    0.615352416876704, 0.775109896436334, 0.355568691389635, 0.405849972041324, 0.706646913895383, 0.838287665275857, 0.239589131204411,
    0.770771533250809, 0.355897744419053, 0.535597037756816, 0.0930881295353174, 0.169803041499108, 0.899832450784743, 0.422637606970966,
    0.747746467823163, 0.822652579983696, 0.95465364633128, 0.685444509377703, 0.500503229675815, 0.275483862496912, 0.228903944836929,
    0.0144339059479535, 0.728964562527835, 0.249880471732467, 0.161183276679367, 0.0170426501426846, 0.486100345151499, 0.102900171885267,
    0.801547004608437, 0.354328064946458, 0.936432539252564, 0.245866392273456, 0.473141461377963, 0.191560871200636, 0.583221969893202,
    0.459473189897835, 0.467434047954157, 0.399832555558532, 0.505285596242175, 0.0318881559651345, 0.114467588020489, 0.468935475917533,
    0.396986737614498, 0.833619194105268, 0.761121743358672, 0.573356448905542, 0.447508045937866, 0.0838020080700517, 0.219138547312468,
    0.07557029183954, 0.534426784375682, 0.641356576699764, 0.525739317527041, 0.0392813880462199, 0.545859839767218, 0.372763097286224,
    0.961302414536476, 0.257341569056734, 0.207951683318242, 0.861382439732552, 0.464391982648522, 0.222867433447391, 0.623549601528794,
    0.203647700604051, 0.0196734135970473, 0.797993005951867, 0.274318896699697, 0.166609104024246, 0.170151718193665, 0.4885059366934,
    0.711409077281132, 0.591934921452776, 0.517156876856461, 0.381627685856074, 0.834778329590335, 0.249008050654083, 0.540601027896628,
    0.743839403614402, 0.0300118550658226
};

std::vector<double> test_y {
    -0.400637547031742, -0.334556565073352, 1.36795395319196, 2.13776710365012, 0.505819264529025, 0.786342384239158, -0.902211944178638,
    0.532896992328326, -0.645894253549207, 0.290987488429766, -1.23759446887722, -0.456176275117809, -0.830322654724733, 0.340115643674263,
    1.06637639568217, 1.2161258380798, 0.735690657633052, -0.48120861731558, 0.562744762858116, -1.2463197118892, 0.380922212625677, -1.43042725279669,
    -1.0484455048786, -0.218503550534591, -1.48993623673554, 1.17270628121431, -1.47982702157166, -0.430387816077224, -1.05163864204345, 1.5225863440541,
    0.59282805458608, -0.222661509019268, 0.712894276248457, 0.716600833741054, 0.440241864384138, 0.158830621318104, 0.659764138331946,
    2.22051966293556, -1.1839450740654, -0.0739558344974904, -0.416354674886519, -0.191482343752627, 0.0695447814074023, 1.15534831801052,
    0.594957346950486, -1.41964510835699, -1.60667724535804, 0.892925899563175, 0.148167955187725, 1.22702839010139, -0.761804339178029,
    0.419375405889908, -1.03994336463235, 0.711573965992631, -0.633213014967826, 0.563174664450148, 0.660986685831601, -1.65805085732545,
    1.02816797701792, 1.12795361401459, -1.28015460342218, 1.12886822740957, -0.464134527164978, -0.315760209531366, 0.924293146834949,
    0.0771447239857784, 1.03992360511188, 0.74188620673818, 1.25554485828952, 0.950918966456178, -0.481365607273294, 0.202881777969837,
    -0.0317397438377262, -1.19558030033457, 0.623681236848433, -0.91480448366691, 0.248758007708097, -1.06262279318038, -0.363982247195756,
    -1.20699485337827, 1.42921278138977, 0.633435890982831, -1.99681561765642, -0.681832173096207, -0.460055479310703, -0.983069194147765,
    0.495331712888337, 0.725817500232534, 0.667298731892919, 0.954786436466587, -1.67533217929194, -1.20518539249191, -1.96325248922053,
    1.47075230981397, 0.372472338550601, 1.06587933403768, 0.530649868357319, 0.101983445884131, 1.33778246578648, 0.0872347684911337
};

std::vector<double> expected_fitted {
    0.114213678964209, 0.0518720629146803, 0.332560802291861, -0.103769372970859, -0.0398498822267076, -0.121115104119403, -0.0729972669213431,
    -0.0798484732459188, -0.292099245811146, 0.330052605608625, -0.186809709595578, -0.217382431222906, 0.0438311405457231, -0.336163698169015,
    0.214726951448916, 0.330812629885827, -0.144110553611766, -0.0978889717807663, 0.348076150908429, 0.382007622915137, 0.440832332823907,
    -0.291016006403793, 0.22635278377455, 0.205587035975013, 0.342953899476573, -0.0616372414930213, 0.386502575890583, -0.13522151002429,
    0.206911254910155, 0.207138481983957, -0.0163046241024913, -0.0166681364163696, -0.0066392986986985, 0.514018650993241, 0.338065972383843,
    0.104177103091272, 0.353242382302481, 0.63088655134516, -0.130112418194629, 0.138161749576699, -0.0698131210238397, -0.125762138300122,
    -0.290629048044196, 0.0213266260210756, -0.129009098051059, 0.0112992372548095, -0.279046696100952, 0.203585777769574, 0.0119071849600766,
    0.304355839495086, 0.199718687089216, 0.592000817056689, -0.134439443949283, 0.248029529323118, -0.0504644299155079, -0.299651340841561,
    0.289829407105298, 0.265391275067946, 0.347271405948945, 0.123421281563047, -0.217337734300595, 0.0468163359149752, 0.25962666036019,
    0.349164796839949, 0.376727764379718, 0.162492726777371, -0.24627911107614, 0.317885617149574, -0.0437645401565862, -0.108577873620113,
    -0.0682396938875849, -0.00839636513260289, -0.197380019278754, 0.0319590613899174, -0.189107640646998, -0.0788034691556874, 0.277430683232674,
    0.645024185645244, -0.106270648836335, -0.0826831910747909, 0.43397377113355, 0.274199306876858, -0.115670997466252, -0.264072251848311,
    -0.0718471520275187, -0.267592607508237, 0.295321708797815, -0.0736844824236668, -0.00045416461248049, -0.00729476177792659, 0.193927052867281,
    -0.0457133979908004, -0.327618445434272, 0.0753904306105746, 0.312021253219726, 0.379165422722196, -0.131520733736989, -0.0434889662178759,
    0.0883843282310566, -0.224754546556252
};

std::vector<double> expected_weights {
    0.971599620988373, 0.983950445608026, 0.887657979553809, 0.531286436342674, 0.968126220290229, 0.913109371061261, 0.927175161448503,
    0.959893364396309, 0.986537929078864, 0.99983532111599, 0.884396338131644, 0.993855944051794, 0.919238111043063, 0.951254131983506,
    0.923260866921085, 0.917207746558554, 0.91821354795349, 0.984206633976232, 0.995033226423986, 0.734338968712607, 0.999612710377881,
    0.864806881993946, 0.832316752758745, 0.980685547371007, 0.670322591658527, 0.842340230340678, 0.659437927928333, 0.990620296619962,
    0.836374223877101, 0.821982846233657, 0.96036014419938, 0.995426100994594, 0.944910240457384, 0.995576165051615, 0.998873705219621,
    0.999677686287345, 0.989886583222715, 0.745898808119225, 0.883745467820188, 0.99515042833184, 0.987082489407359, 0.999533958330217,
    0.986049838505151, 0.866036942451845, 0.944239124644507, 0.791241595253004, 0.818835491265111, 0.949377797312572, 0.997997365859249,
    0.910240222366471, 0.902719181277304, 0.996786800463788, 0.913474628759808, 0.976946579786438, 0.963688621373872, 0.921275021256666,
    0.98518927093793, 0.640606866873204, 0.950594702887476, 0.894070138694254, 0.881817352242057, 0.877641499197606, 0.944270285398732,
    0.952857618779474, 0.967906118032582, 0.999214078981764, 0.829443791076373, 0.980693704052802, 0.826116627279935, 0.882531529838827,
    0.981666808251773, 0.995188689894951, 0.997041389205959, 0.844000111773879, 0.929979865928298, 0.926001272993224, 0.999911283512059,
    0.710073079084261, 0.992845708970396, 0.868240530183955, 0.895967477494426, 0.986122093010904, 0.654581648281221, 0.981255173366129,
    0.983802877434222, 0.945521044229265, 0.995687669812592, 0.932210567087844, 0.952460670402226, 0.902609097232298, 0.65847923593018,
    0.860184990862303, 0.732135197818656, 0.800924596541718, 0.999605685343433, 0.949757816512891, 0.953242711498773, 0.997717605073975,
    0.838640928517098, 0.989523529091164    
};

std::vector<double> expected_fitted_10 {
    0.0634198110474751, 0.0898659370695178, 0.25508116384502, -0.095458499182449, -0.0538728114835423, -0.0915724075017522, 0.0179481898045382,
    0.0129835542567723, -0.17515766030118, 0.257332381318116, -0.148855315118655, -0.210464644196017, 0.0415471381868463, -0.189770419106098,
    0.198995220810087, 0.256630178103506, -0.165635966612415, -0.00514736325899556, 0.23217936716617, 0.378038558213406, 0.439405190774132,
    -0.175507775424826, 0.210726191541624, 0.196974166534719, 0.23792925784833, -0.0448874592192849, 0.38266142890071, -0.0600004268218819,
    0.19452846233058, 0.197242186694353, -0.0643825334935659, -0.0271292730381942, -0.0368851631891207, 0.514776913278761, 0.25160311644087,
    0.108561990895465, 0.349098278918367, 0.631624074891777, -0.124048836981818, 0.09512162148823, 0.0200749366759084, -0.0838373564103895,
    -0.291647602120835, 0.0384377981915804, -0.0370421038594311, -0.0248755835004792, -0.282874256915439, 0.160583879674883, 0.00586914623186402,
    0.303791868460941, 0.195963651913615, 0.592787186670551, -0.0459968685342266, 0.219483039350977, -0.0671994925376371, -0.199096722971895,
    0.281606422697338, 0.2454236882881, 0.233027953857448, 0.0733853842517411, -0.232948032798824, 0.0402115502428526, 0.23859957851889,
    0.230709979210636, 0.372639827153178, 0.158499970262922, -0.206339611849908, 0.271860577780307, -0.0583589924558958, -0.105622385252074,
    -0.0860426910987137, -0.05906363859788, -0.156416514030712, -0.0195784093110896, -0.208084200859314, -0.111027786657211, 0.210979348074471,
    0.645795412916327, -0.0203975975817678, -0.0900361671342712, 0.432237965543405, 0.259250117578841, -0.0973038409436459, -0.169489715393135,
    -0.0840395971041663, -0.274026859771066, 0.296162655845942, 0.0174760856380117, -0.0324351755819134, -0.0373709607295901, 0.149650274372298,
    -0.0271074279765164, -0.192700006566908, 0.0194294676249935, 0.218199729688271, 0.375128092321434, -0.0389883347534711, -0.0871260675365439,
    0.0939745611766192, -0.239258132321846
};

std::vector<double> expected_fitted_m1 {
    0.113826504573302, 0.0519008474491234, 0.332039811396697, -0.103399583673586, -0.0703828802757206, -0.12150728189891, -0.0730477299916967,
    -0.0799124642640797, -0.291763936878344, 0.329506160044385, -0.186433024627222, -0.21762831970395, 0.0192494383737922, -0.336129722037656,
    0.214676139632408, 0.330274080083479, -0.17759651811647, -0.0980011381260869, 0.347809567696531, 0.381590768531851, 0.440303141140581,
    -0.290683596922588, 0.226247782307221, 0.205535652005553, 0.342623609377804, -0.0613017773614302, 0.386077110073262, -0.135486424940031,
    0.206843161900851, 0.20708579507172, -0.0166127964752836, -0.04594137618155, -0.00795417139646881, 0.513369485477301, 0.337579402802734,
    0.104283202854659, 0.352879339761696, 0.630116385685383, -0.129713138374906, 0.137743403554875, -0.0698577247183755, -0.126124182067339,
    -0.326942828162998, 0.0215484725499624, -0.129203241605725, 0.00987679036414224, -0.315104677682974, 0.203100941742813, -0.0153522269963007,
    0.304079818677933, 0.199672321007832, 0.591260893992169, -0.134659524140655, 0.247493772448264, -0.0514389058828676, -0.299820031465589,
    0.289223459164072, 0.264831091577179, 0.346994699213923, 0.12302668105463, -0.252123730569516, 0.0224923092264272, 0.259076175703167,
    0.348916258261498, 0.376320959070516, 0.162495211370988, -0.246499419755201, 0.31720832159747, -0.0744794943169872, -0.10906771148434,
    -0.0999186623131995, -0.00870532249910361, -0.197011714939222, 0.0316104623447691, -0.223381704082538, -0.0790901193696234, 0.277334971557717,
    0.644245373260999, -0.106422870935907, -0.0833893028558603, 0.433457264299204, 0.273625447509674, -0.116104444466046, -0.263702431931664,
    -0.0726388936761714, -0.303401576888546, 0.295061904772407, -0.0737362406974615, -0.00181015276295687, -0.00860502380027195, 0.193457044955582,
    -0.0454217127313652, -0.327648984002642, 0.0750278305328607, 0.311892086805757, 0.378753986898135, -0.131720418291751, -0.04377972564625,
    0.088527729351215
};

TEST(ReferenceTest, Vanilla) {
    WeightedLowess::WeightedLowess wl;
    auto res = wl.run(test_x.size(), test_x.data(), test_y.data());
    compare_almost_equal(res.fitted, expected_fitted);

    // Checking behavior with interpolation.
    res = wl.set_anchors(10).run(test_x.size(), test_x.data(), test_y.data());
    compare_almost_equal(res.fitted, expected_fitted_10);

    // Checking behavior with odd number of points.
    res = wl.set_anchors(200).run(test_x.size() - 1, test_x.data(), test_y.data());
    compare_almost_equal(res.fitted, expected_fitted_m1);
}

TEST(ReferenceTest, Weights) {
    // Checking weights are correctly computed. Note that limma::weightedLowess
    // recomputes the weights _after_ the last iteration, while WeightedLowess::WeightedLowess
    // reports the weights used _during_ the last iteration. Thus, we need to add 
    // an extra iteration to get the same matching weights.
    WeightedLowess::WeightedLowess wl;
    auto res = wl.set_iterations(4).run(test_x.size(), test_x.data(), test_y.data());
    compare_almost_equal(res.robust_weights, expected_weights);
}
